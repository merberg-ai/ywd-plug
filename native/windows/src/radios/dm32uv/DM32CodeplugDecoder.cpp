#include "DM32CodeplugDecoder.h"

#include "DM32Constants.h"

#include <QFile>
#include <QHash>

#include <algorithm>

namespace {
constexpr quint8 ScanListMetadata = 0x11;
constexpr quint8 RxGroupsMetadata = 0x0F;
constexpr quint8 ZoneFirstMetadata = 0x5C;
constexpr quint8 ZoneLastMetadata = 0x64;
constexpr int ZoneSize = 145;
constexpr int ZonesPerBlock = 28;
constexpr int MaximumZones = 250;
constexpr int ScanListSize = 57;
constexpr int MaximumScanLists = 32;
constexpr int RxGroupEntryStart = 0x11;
constexpr int RxGroupEntrySize = 109;
constexpr int MaximumRxGroups = 32;
constexpr int RxGroupMemberSlots = 32;

quint8 byteValue(char value)
{
    return static_cast<quint8>(static_cast<unsigned char>(value));
}

QString decodeAscii(const QByteArray &data, int offset, int length)
{
    if (offset < 0 || length <= 0 || offset >= data.size()) {
        return {};
    }

    const int boundedLength = qMin(length, data.size() - offset);
    const QByteArray field = data.mid(offset, boundedLength);
    if (field.isEmpty() || byteValue(field.at(0)) == 0xFF) {
        return {};
    }

    const int nullIndex = field.indexOf('\0');
    QByteArray trimmed = nullIndex >= 0 ? field.left(nullIndex) : field;

    while (!trimmed.isEmpty()
           && (byteValue(trimmed.back()) == 0x00 || byteValue(trimmed.back()) == 0xFF)) {
        trimmed.chop(1);
    }

    return QString::fromLatin1(trimmed).trimmed();
}

quint16 readUint16LE(const QByteArray &data, int offset)
{
    if (offset < 0 || offset + 1 >= data.size()) {
        return 0;
    }
    return static_cast<quint16>(byteValue(data.at(offset)))
        | (static_cast<quint16>(byteValue(data.at(offset + 1))) << 8);
}

quint32 readUint24LE(const QByteArray &data, int offset)
{
    if (offset < 0 || offset + 2 >= data.size()) {
        return 0;
    }
    return static_cast<quint32>(byteValue(data.at(offset)))
        | (static_cast<quint32>(byteValue(data.at(offset + 1))) << 8)
        | (static_cast<quint32>(byteValue(data.at(offset + 2))) << 16);
}

QHash<int, QByteArray> blockMap(const QVector<DM32MemoryBlock> &blocks)
{
    QHash<int, QByteArray> mapped;
    for (const DM32MemoryBlock &block : blocks) {
        if (block.data.size() != DM32Constants::BlockSize) {
            continue;
        }
        if (!mapped.contains(block.metadata)) {
            mapped.insert(block.metadata, block.data);
        }
    }
    return mapped;
}

QVector<DM32ZoneInfo> decodeZones(const QHash<int, QByteArray> &blocks)
{
    QByteArray allZoneData;
    for (int metadata = ZoneFirstMetadata; metadata <= ZoneLastMetadata; ++metadata) {
        const auto it = blocks.constFind(metadata);
        if (it == blocks.constEnd()) {
            break;
        }
        allZoneData.append(it.value());
    }

    QVector<DM32ZoneInfo> zones;
    if (allZoneData.isEmpty()) {
        return zones;
    }

    for (int zoneNumber = 1; zoneNumber <= MaximumZones; ++zoneNumber) {
        const int zoneIndex = zoneNumber - 1;
        const int blockIndex = zoneIndex / ZonesPerBlock;
        const int indexInBlock = zoneIndex % ZonesPerBlock;
        const int offset = blockIndex == 0
            ? 16 + indexInBlock * ZoneSize
            : blockIndex * DM32Constants::BlockSize + indexInBlock * ZoneSize;

        if (offset + ZoneSize > allZoneData.size()) {
            break;
        }

        const QByteArray entry = allZoneData.mid(offset, ZoneSize);
        const QString name = decodeAscii(entry, 0, 11);
        if (name.isEmpty()) {
            continue;
        }

        DM32ZoneInfo zone;
        zone.number = zoneNumber;
        zone.name = name;

        const int declaredCount = byteValue(entry.at(16));
        const int maximumToRead = declaredCount > 0 && declaredCount <= 64
            ? declaredCount
            : 64;

        for (int index = 0; index < maximumToRead; ++index) {
            const int channelOffset = 17 + index * 2;
            const quint16 channel = readUint16LE(entry, channelOffset);
            if (channel == 0 || channel == 0xFFFF) {
                break;
            }
            if (channel < 1 || channel > 4000) {
                break;
            }
            zone.channels.push_back(static_cast<int>(channel));
        }

        zones.push_back(zone);
    }

    return zones;
}

QVector<DM32ScanListInfo> decodeScanLists(const QHash<int, QByteArray> &blocks)
{
    QVector<DM32ScanListInfo> scanLists;
    const auto it = blocks.constFind(ScanListMetadata);
    if (it == blocks.constEnd()) {
        return scanLists;
    }

    const QByteArray &data = it.value();
    if (data.isEmpty()) {
        return scanLists;
    }

    const int count = qMin<int>(byteValue(data.at(0)), MaximumScanLists);
    for (int listNumber = 1; listNumber <= count; ++listNumber) {
        const int entryOffset = (ScanListSize * listNumber) - 56;
        if (entryOffset + ScanListSize > data.size()) {
            break;
        }

        const QByteArray entry = data.mid(entryOffset, ScanListSize);
        DM32ScanListInfo list;
        list.number = listNumber;
        list.name = decodeAscii(entry, 0, 11);
        if (list.name.isEmpty()) {
            list.name = QStringLiteral("Scan List %1").arg(listNumber);
        }

        list.hangTimeSeconds = static_cast<double>(byteValue(entry.at(0x0D))) / 10.0;

        const quint8 mode = byteValue(entry.at(0x0C));
        list.designatedTxMode = static_cast<int>((mode >> 2) & 0x03U);

        const quint8 priorityTypes = byteValue(entry.at(0x0E));
        list.priority1Type = static_cast<int>(priorityTypes & 0x0FU);
        list.priority2Type = static_cast<int>((priorityTypes >> 4) & 0x0FU);

        const quint16 priority1Raw = readUint16LE(entry, 0x0F);
        if (list.priority1Type == 2 && priority1Raw > 0) {
            list.priority1Channel = static_cast<int>(priority1Raw);
        }

        const quint16 designatedTxRaw = readUint16LE(entry, 0x11);
        if (list.designatedTxMode == 2) {
            list.designatedTxChannel = static_cast<int>(designatedTxRaw) + 2;
        }

        const quint16 priority2Raw = readUint16LE(entry, 0x13);
        if (list.priority2Type >= 2) {
            list.priority2Channel = static_cast<int>(priority2Raw) + 2;
        }

        for (int index = 0; index < 15; ++index) {
            const quint16 channel = readUint16LE(entry, 0x1A + index * 2);
            if (channel == 0 || channel == 0xFFFF) {
                break;
            }
            list.channels.push_back(static_cast<int>(channel));
        }

        scanLists.push_back(list);
    }

    return scanLists;
}

QVector<DM32RxGroupInfo> decodeRxGroups(const QHash<int, QByteArray> &blocks)
{
    QVector<DM32RxGroupInfo> groups;
    const auto it = blocks.constFind(RxGroupsMetadata);
    if (it == blocks.constEnd()) {
        return groups;
    }

    const QByteArray &data = it.value();
    if (data.size() < RxGroupEntryStart) {
        return groups;
    }

    const quint32 bitmask = static_cast<quint32>(byteValue(data.at(0)))
        | (static_cast<quint32>(byteValue(data.at(1))) << 8)
        | (static_cast<quint32>(byteValue(data.at(2))) << 16)
        | (static_cast<quint32>(byteValue(data.at(3))) << 24);

    int activeCount = 0;
    for (int bit = 0; bit < MaximumRxGroups; ++bit) {
        if ((bitmask & (1U << bit)) != 0U) {
            activeCount = bit + 1;
        }
    }

    for (int index = 0; index < activeCount; ++index) {
        const int entryOffset = RxGroupEntryStart + index * RxGroupEntrySize;
        if (entryOffset + RxGroupEntrySize > data.size()) {
            break;
        }

        const QByteArray entry = data.mid(entryOffset, RxGroupEntrySize);
        DM32RxGroupInfo group;
        group.number = index + 1;
        group.name = decodeAscii(entry, 0, 11);
        if (group.name.isEmpty()) {
            group.name = QStringLiteral("RX Group %1").arg(group.number);
        }

        for (int member = 0; member < RxGroupMemberSlots; ++member) {
            const int memberOffset = 11 + member * 3;
            const quint32 id = readUint24LE(entry, memberOffset);
            if (id == 0 || id == 0x00FFFFFFU) {
                continue;
            }
            group.memberIds.push_back(id);
        }

        groups.push_back(group);
    }

    return groups;
}

QVector<DM32MemoryBlock> blocksFromRawImage(const QByteArray &raw)
{
    QVector<DM32MemoryBlock> blocks;
    if (raw.isEmpty() || raw.size() % DM32Constants::BlockSize != 0) {
        return blocks;
    }

    const int count = raw.size() / DM32Constants::BlockSize;
    blocks.reserve(count);
    for (int index = 0; index < count; ++index) {
        const QByteArray data = raw.mid(index * DM32Constants::BlockSize, DM32Constants::BlockSize);
        const quint8 metadata = byteValue(data.at(DM32Constants::BlockSize - 1));
        blocks.push_back({static_cast<quint32>(index * DM32Constants::BlockSize), metadata, data});
    }
    return blocks;
}
} // namespace

DM32CodeplugDecodeResult DM32CodeplugDecoder::decodeBlocks(const QVector<DM32MemoryBlock> &blocks)
{
    DM32CodeplugDecodeResult result;
    if (blocks.isEmpty()) {
        result.error = QStringLiteral("No memory blocks were supplied to the codeplug decoder");
        return result;
    }

    const QHash<int, QByteArray> mapped = blockMap(blocks);
    result.zones = decodeZones(mapped);
    result.scanLists = decodeScanLists(mapped);
    result.rxGroups = decodeRxGroups(mapped);
    result.ok = true;
    return result;
}

DM32CodeplugDecodeResult DM32CodeplugDecoder::decodeFile(const QString &path)
{
    DM32CodeplugDecodeResult result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("Could not open raw backup: %1").arg(path);
        return result;
    }

    const QByteArray raw = file.readAll();
    const QVector<DM32MemoryBlock> blocks = blocksFromRawImage(raw);
    if (blocks.isEmpty()) {
        result.error = QStringLiteral("Raw backup is not a whole-number multiple of 4096 bytes");
        return result;
    }

    return decodeBlocks(blocks);
}
