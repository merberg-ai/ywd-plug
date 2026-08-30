#include "DM32ContactDecoder.h"

#include "DM32Constants.h"

#include <QHash>
#include <QSet>

#include <algorithm>

namespace {
constexpr int ContactEntrySize = 0x5C;
constexpr int ContactsPerBlock = 44;
constexpr int FirstBlockHeaderSize = 0x10;

quint8 byteValue(char value)
{
    return static_cast<quint8>(static_cast<unsigned char>(value));
}

quint32 readUint32LE(const QByteArray &data, qsizetype offset)
{
    return static_cast<quint32>(byteValue(data.at(offset)))
        | (static_cast<quint32>(byteValue(data.at(offset + 1))) << 8)
        | (static_cast<quint32>(byteValue(data.at(offset + 2))) << 16)
        | (static_cast<quint32>(byteValue(data.at(offset + 3))) << 24);
}

QString fixedAscii(const QByteArray &record, qsizetype offset, qsizetype length)
{
    if (offset < 0 || length <= 0 || offset + length > record.size()) {
        return {};
    }

    QByteArray field = record.mid(offset, length);
    qsizetype end = 0;
    while (end < field.size()) {
        const quint8 value = byteValue(field.at(end));
        if (value == 0x00 || value == 0xFF) {
            break;
        }
        ++end;
    }
    field.truncate(end);
    return QString::fromLatin1(field).trimmed();
}
} // namespace

DM32ContactDecodeResult DM32ContactDecoder::decodeReferenced(
    const QVector<DM32ContactBlock> &blocks,
    quint32 contactBase,
    int databaseCount,
    const QVector<int> &referencedContactIds)
{
    DM32ContactDecodeResult result;
    result.databaseCount = databaseCount;

    if (referencedContactIds.isEmpty()) {
        result.ok = true;
        return result;
    }

    if (contactBase == 0) {
        result.error = QStringLiteral("Contact base address is zero");
        return result;
    }

    QHash<int, const DM32ContactBlock *> byBlockNumber;
    for (const DM32ContactBlock &block : blocks) {
        if (block.data.size() != DM32Constants::BlockSize) {
            result.error = QStringLiteral("Contact block %1 at 0x%2 has %3 bytes; expected 4096")
                               .arg(block.blockNumber)
                               .arg(block.address, 6, 16, QLatin1Char('0'))
                               .arg(block.data.size())
                               .toUpper();
            return result;
        }
        byBlockNumber.insert(block.blockNumber, &block);
    }

    QVector<int> ids = referencedContactIds;
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

    const quint32 firstBlockAddress = contactBase & ~0x0FFFU;
    const qsizetype baseOffset = static_cast<qsizetype>(contactBase - firstBlockAddress);

    for (const int contactId : ids) {
        if (contactId <= 0) {
            continue;
        }
        if (databaseCount > 0 && contactId > databaseCount) {
            continue;
        }

        const int zeroBased = contactId - 1;
        const int blockNumber = zeroBased / ContactsPerBlock;
        const int indexInBlock = zeroBased % ContactsPerBlock;

        const DM32ContactBlock *block = byBlockNumber.value(blockNumber, nullptr);
        if (!block) {
            result.error = QStringLiteral("Referenced contact %1 requires contact block %2, but that block was not read")
                               .arg(contactId)
                               .arg(blockNumber);
            result.contacts.clear();
            return result;
        }

        const qsizetype entryOffset = blockNumber == 0
            ? baseOffset + FirstBlockHeaderSize + static_cast<qsizetype>(indexInBlock * ContactEntrySize)
            : static_cast<qsizetype>(indexInBlock * ContactEntrySize);

        if (entryOffset < 0 || entryOffset + ContactEntrySize > block->data.size()) {
            result.error = QStringLiteral("Referenced contact %1 points outside contact block %2")
                               .arg(contactId)
                               .arg(blockNumber);
            result.contacts.clear();
            return result;
        }

        const QByteArray record = block->data.mid(entryOffset, ContactEntrySize);
        const quint8 first = byteValue(record.at(0));
        const quint32 dmrId = readUint32LE(record, 0x10);
        if ((first == 0x00 || first == 0xFF) && dmrId == 0) {
            continue;
        }

        DM32ContactInfo contact;
        contact.index = contactId;
        contact.name = fixedAscii(record, 0x00, 16);
        contact.dmrId = dmrId;
        contact.callSign = fixedAscii(record, 0x14, 8);
        contact.city = fixedAscii(record, 0x1C, 16);
        contact.province = fixedAscii(record, 0x2C, 16);
        contact.country = fixedAscii(record, 0x3C, 16);
        contact.remark = fixedAscii(record, 0x4C, 16);

        if (contact.name.isEmpty()) {
            contact.name = contact.callSign.isEmpty()
                ? QStringLiteral("Contact %1").arg(contactId)
                : contact.callSign;
        }

        result.contacts.push_back(contact);
    }

    result.ok = true;
    return result;
}
