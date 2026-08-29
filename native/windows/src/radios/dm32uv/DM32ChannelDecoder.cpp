#include "DM32ChannelDecoder.h"

#include "DM32Constants.h"

#include <QByteArray>
#include <QFile>

#include <algorithm>

namespace {
constexpr quint8 ChannelFirstMetadata = 0x12;
constexpr quint8 ChannelLastMetadata = 0x41;
constexpr quint8 TxContactLowMetadata = 0x42;
constexpr qsizetype ChannelSize = 48;
constexpr qsizetype FirstChannelOffset = 0x10;
constexpr int ChannelsInFirstBlock = 84;
constexpr int ChannelsInFollowingBlock = 85;
constexpr int MaximumChannels = 4000;

struct BlockRef
{
    quint8 metadata {0};
    qsizetype offset {0};
};

quint8 byteValue(char value)
{
    return static_cast<quint8>(static_cast<unsigned char>(value));
}

QString decodeName(const QByteArray &record)
{
    QByteArray name = record.left(16);
    const qsizetype nullIndex = name.indexOf('\0');
    if (nullIndex >= 0) {
        name.truncate(nullIndex);
    }

    while (!name.isEmpty() && byteValue(name.back()) == 0xFF) {
        name.chop(1);
    }

    return QString::fromLatin1(name).trimmed();
}

double decodeBcdFrequency(const QByteArray &record, qsizetype offset)
{
    const int order[] = {3, 2, 1, 0};
    qint64 frequencyInteger = 0;

    for (const int index : order) {
        const quint8 value = byteValue(record.at(offset + index));
        const int high = (value >> 4) & 0x0F;
        const int low = value & 0x0F;
        frequencyInteger = (frequencyInteger * 100) + (high * 10) + low;
    }

    return static_cast<double>(frequencyInteger) / 100000.0;
}

QString modeName(quint8 modeFlags)
{
    switch ((modeFlags >> 4) & 0x0F) {
    case 0:
        return QStringLiteral("Analog");
    case 1:
        return QStringLiteral("Digital");
    case 2:
        return QStringLiteral("Fixed Analog");
    case 3:
        return QStringLiteral("Fixed Digital");
    default:
        return QStringLiteral("Analog");
    }
}

QString powerName(quint8 modeFlags)
{
    switch ((modeFlags >> 1) & 0x03) {
    case 0:
        return QStringLiteral("Low");
    case 1:
        return QStringLiteral("Medium");
    case 2:
        return QStringLiteral("High");
    default:
        return QStringLiteral("Low");
    }
}

bool isDigitalMode(const QString &mode)
{
    return mode == QStringLiteral("Digital") || mode == QStringLiteral("Fixed Digital");
}
}

DM32ChannelDecodeResult DM32ChannelDecoder::decodeFile(const QString &path)
{
    DM32ChannelDecodeResult result;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("Could not open raw image: %1").arg(file.errorString());
        return result;
    }

    const QByteArray image = file.readAll();
    if (image.isEmpty()) {
        result.error = QStringLiteral("Raw image is empty");
        return result;
    }

    if ((image.size() % DM32Constants::BlockSize) != 0) {
        result.error = QStringLiteral("Raw image size %1 is not a multiple of the 4096-byte DM-32UV block size")
                           .arg(image.size());
        return result;
    }

    QVector<BlockRef> channelBlocks;
    qsizetype txContactLowOffset = -1;

    for (qsizetype blockOffset = 0;
         blockOffset + DM32Constants::BlockSize <= image.size();
         blockOffset += DM32Constants::BlockSize) {
        const quint8 metadata = byteValue(image.at(blockOffset + DM32Constants::BlockSize - 1));

        if (metadata >= ChannelFirstMetadata && metadata <= ChannelLastMetadata) {
            channelBlocks.push_back({metadata, blockOffset});
        } else if (metadata == TxContactLowMetadata) {
            txContactLowOffset = blockOffset;
        }
    }

    std::sort(channelBlocks.begin(), channelBlocks.end(), [](const BlockRef &left, const BlockRef &right) {
        return left.metadata < right.metadata;
    });

    const auto firstBlockIt = std::find_if(channelBlocks.cbegin(), channelBlocks.cend(), [](const BlockRef &block) {
        return block.metadata == ChannelFirstMetadata;
    });
    if (firstBlockIt == channelBlocks.cend()) {
        result.error = QStringLiteral("Channel block metadata 0x12 was not found in the raw image");
        return result;
    }

    const qsizetype firstBlockOffset = firstBlockIt->offset;
    const int channelCount = byteValue(image.at(firstBlockOffset))
        | (static_cast<int>(byteValue(image.at(firstBlockOffset + 1))) << 8);

    if (channelCount < 0 || channelCount > MaximumChannels) {
        result.error = QStringLiteral("Invalid channel count %1 in metadata block 0x12").arg(channelCount);
        return result;
    }

    result.channelCount = channelCount;
    if (channelCount == 0) {
        result.ok = true;
        return result;
    }

    const int remainingChannels = qMax(0, channelCount - ChannelsInFirstBlock);
    const int requiredBlocks = 1
        + ((remainingChannels + ChannelsInFollowingBlock - 1) / ChannelsInFollowingBlock);

    if (channelBlocks.size() < requiredBlocks) {
        result.error = QStringLiteral("Channel count %1 requires %2 logical channel blocks, but only %3 were found")
                           .arg(channelCount)
                           .arg(requiredBlocks)
                           .arg(channelBlocks.size());
        return result;
    }

    result.channels.reserve(channelCount);

    for (int channelNumber = 1; channelNumber <= channelCount; ++channelNumber) {
        int logicalBlockIndex = 0;
        qsizetype recordOffsetInBlock = 0;

        if (channelNumber <= ChannelsInFirstBlock) {
            logicalBlockIndex = 0;
            recordOffsetInBlock = FirstChannelOffset
                + static_cast<qsizetype>(channelNumber - 1) * ChannelSize;
        } else {
            const int followingIndex = channelNumber - ChannelsInFirstBlock - 1;
            logicalBlockIndex = 1 + (followingIndex / ChannelsInFollowingBlock);
            const int recordIndex = followingIndex % ChannelsInFollowingBlock;
            recordOffsetInBlock = static_cast<qsizetype>(recordIndex) * ChannelSize;
        }

        const BlockRef &block = channelBlocks.at(logicalBlockIndex);
        const qsizetype absoluteOffset = block.offset + recordOffsetInBlock;
        if (absoluteOffset < 0 || absoluteOffset + ChannelSize > image.size()) {
            result.error = QStringLiteral("Channel %1 points outside the captured image").arg(channelNumber);
            result.channels.clear();
            return result;
        }

        const QByteArray record = image.mid(absoluteOffset, ChannelSize);
        DM32ChannelInfo channel;
        channel.number = channelNumber;
        channel.name = decodeName(record);
        if (channel.name.isEmpty()) {
            channel.name = QStringLiteral("Channel %1").arg(channelNumber);
        }

        channel.rxFrequency = decodeBcdFrequency(record, 0x10);

        channel.txDisabled = true;
        for (qsizetype index = 0x14; index < 0x18; ++index) {
            if (byteValue(record.at(index)) != 0xFF) {
                channel.txDisabled = false;
                break;
            }
        }
        if (!channel.txDisabled) {
            channel.txFrequency = decodeBcdFrequency(record, 0x14);
        }

        const quint8 modeFlags = byteValue(record.at(0x18));
        channel.mode = modeName(modeFlags);
        channel.power = powerName(modeFlags);

        const quint8 scanBandwidth = byteValue(record.at(0x19));
        channel.bandwidth = (scanBandwidth & 0x80) != 0
            ? QStringLiteral("25kHz")
            : QStringLiteral("12.5kHz");
        channel.scanAdd = (scanBandwidth & 0x40) != 0;
        channel.scanListId = (scanBandwidth >> 2) & 0x0F;

        if (isDigitalMode(channel.mode)) {
            const quint8 digitalFeatures = byteValue(record.at(0x1D));
            channel.timeSlot = (digitalFeatures & 0x10) != 0 ? 2 : 1;
            channel.colorCode = digitalFeatures & 0x0F;
            channel.rxGroupListId = byteValue(record.at(0x1F)) & 0x3F;
        }

        if (txContactLowOffset >= 0 && channelNumber <= 2047) {
            const qsizetype contactOffset = txContactLowOffset
                + static_cast<qsizetype>(channelNumber - 1) * 2;
            if (contactOffset + 1 < image.size()) {
                const quint8 byte0 = byteValue(image.at(contactOffset));
                const quint8 byte1 = byteValue(image.at(contactOffset + 1));
                channel.txContactIndex = ((byte0 >> 4) & 0x0F) * 256 + byte1;
                channel.txContactDigital = (byte0 & 0x01) != 0;
            }
        }

        result.channels.push_back(channel);
    }

    result.ok = true;
    return result;
}
