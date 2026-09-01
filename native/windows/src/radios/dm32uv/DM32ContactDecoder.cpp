#include "DM32ContactDecoder.h"

#include "DM32Constants.h"

#include <QHash>
#include <QSet>

#include <algorithm>

namespace {
constexpr int TalkGroupNameSize = 16;
constexpr int TalkGroupEntrySize = 24;
constexpr int TalkGroupHeaderSize = 1;

quint8 byteValue(char value)
{
    return static_cast<quint8>(static_cast<unsigned char>(value));
}

QString asciiName(const QByteArray &data, qsizetype offset)
{
    if (offset < 0 || offset + TalkGroupNameSize > data.size()) {
        return {};
    }

    QByteArray field = data.mid(offset, TalkGroupNameSize);
    qsizetype length = 0;
    while (length < field.size()) {
        const quint8 value = byteValue(field.at(length));
        if (value == 0x00 || value == 0xFF) {
            break;
        }
        ++length;
    }
    field.truncate(length);
    return QString::fromLatin1(field).trimmed();
}

QString callTypeText(quint8 callType)
{
    switch (callType) {
    case 0x03:
        return QStringLiteral("PRIVATE");
    case 0x04:
        return QStringLiteral("GROUP");
    case 0x05:
        return QStringLiteral("ALL");
    default:
        return QStringLiteral("TYPE 0x%1")
            .arg(QString::number(callType, 16).toUpper().rightJustified(2, QLatin1Char('0')));
    }
}
} // namespace

DM32ContactDecodeResult DM32ContactDecoder::decodeReferenced(
    const QVector<DM32ContactBlock> &blocks,
    quint32 contactBase,
    int databaseCount,
    const QVector<int> &referencedContactIds)
{
    Q_UNUSED(contactBase);

    DM32ContactDecodeResult result;
    result.databaseCount = databaseCount;

    if (referencedContactIds.isEmpty()) {
        result.ok = true;
        return result;
    }

    if (blocks.isEmpty()) {
        result.error = QStringLiteral("Talk group block 0x44 was not read");
        return result;
    }

    const QByteArray &data = blocks.constFirst().data;
    if (data.size() != DM32Constants::BlockSize) {
        result.error = QStringLiteral("Talk group block has %1 bytes; expected %2")
                           .arg(data.size())
                           .arg(DM32Constants::BlockSize);
        return result;
    }

    QSet<int> wanted;
    int highestWanted = 0;
    for (const int id : referencedContactIds) {
        if (id > 0) {
            wanted.insert(id);
            highestWanted = qMax(highestWanted, id);
        }
    }

    if (wanted.isEmpty()) {
        result.ok = true;
        return result;
    }

    qsizetype offset = 0;
    int contactIndex = 1;

    // The browser driver and captured codeplugs both use a single 0x00 header
    // byte before contact 1. It is not part of the normal 24-byte entries.
    if (!data.isEmpty() && byteValue(data.at(0)) == 0x00) {
        offset += TalkGroupHeaderSize;
    }

    // Leave the metadata byte at 0xFFF out of the parser.
    const qsizetype payloadEnd = data.size() - 1;

    while (contactIndex <= highestWanted && offset + TalkGroupEntrySize <= payloadEnd) {
        const quint8 flag = byteValue(data.at(offset));
        Q_UNUSED(flag);
        const qsizetype nameOffset = offset + 1;
        const QString name = asciiName(data, nameOffset);

        const qsizetype nullOffset = nameOffset + TalkGroupNameSize;
        const qsizetype contactNumberOffset = nullOffset + 1;
        const qsizetype callTypeOffset = contactNumberOffset + 3;

        const quint32 contactNumber = static_cast<quint32>(byteValue(data.at(contactNumberOffset)))
            | (static_cast<quint32>(byteValue(data.at(contactNumberOffset + 1))) << 8)
            | (static_cast<quint32>(byteValue(data.at(contactNumberOffset + 2))) << 16);
        const quint8 callType = byteValue(data.at(callTypeOffset));

        if (wanted.contains(contactIndex)) {
            const bool empty = name.isEmpty()
                && (contactNumber == 0 || contactNumber == 0x00FFFFFFU);

            if (!empty) {
                DM32ContactInfo contact;
                contact.index = contactIndex;
                contact.name = name.isEmpty()
                    ? QStringLiteral("Contact %1").arg(contactIndex)
                    : name;
                contact.dmrId = contactNumber;
                // Reuse the existing UI field until the model is widened: for
                // codeplug contacts this is the DMR call type, not a callsign.
                contact.callSign = callTypeText(callType);
                result.contacts.push_back(contact);
            }
        }

        offset += TalkGroupEntrySize;
        ++contactIndex;
    }

    std::sort(
        result.contacts.begin(),
        result.contacts.end(),
        [](const DM32ContactInfo &a, const DM32ContactInfo &b) {
            return a.index < b.index;
        });

    result.ok = true;
    return result;
}
