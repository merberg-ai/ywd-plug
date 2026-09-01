#include "DM32Connection.h"

#include "DM32Constants.h"
#include "serial/WinSerialPort.h"

#include <QElapsedTimer>
#include <QHash>
#include <QSet>
#include <QThread>

#include <algorithm>

namespace {
constexpr quint8 ChannelFirstMetadata = 0x12;
constexpr quint8 ChannelLastMetadata = 0x41;
constexpr quint8 TxContactLowMetadata = 0x42;
constexpr quint8 TxContactHighMetadata = 0x43;
constexpr quint8 TalkGroupsMetadata = 0x44;
constexpr quint8 TalkGroupCounterMetadata = 0x06;
constexpr quint8 ScanListMetadata = 0x11;
constexpr quint8 RxGroupsMetadata = 0x0F;
constexpr quint8 ZoneFirstMetadata = 0x5C;
constexpr quint8 ZoneLastMetadata = 0x64;
constexpr int TalkGroupCounterOffset = 0x1FF;
constexpr int ChannelsInFirstBlock = 84;
constexpr int ChannelsInFollowingBlock = 85;
constexpr int MaximumChannels = 4000;

quint8 byteValue(char value)
{
    return static_cast<quint8>(static_cast<unsigned char>(value));
}

QString toHex(const QByteArray &data)
{
    return QString::fromLatin1(data.toHex(' ')).toUpper();
}

QString hexAddress(quint32 address)
{
    return QStringLiteral("0x%1")
        .arg(QString::number(address, 16).toUpper().rightJustified(6, QLatin1Char('0')));
}

QString hexByte(quint8 value)
{
    return QStringLiteral("0x%1")
        .arg(QString::number(value, 16).toUpper().rightJustified(2, QLatin1Char('0')));
}

QString shortReadError(
    const QString &stage,
    qsizetype expected,
    const QByteArray &received,
    const WinSerialPort &port)
{
    if (!port.errorString().isEmpty()) {
        return QStringLiteral("%1 read failed: %2; received %3/%4 bytes [%5]")
            .arg(stage, port.errorString())
            .arg(received.size())
            .arg(expected)
            .arg(toHex(received));
    }

    return QStringLiteral("%1 timeout: expected %2 bytes, received %3 [%4]")
        .arg(stage)
        .arg(expected)
        .arg(received.size())
        .arg(toHex(received));
}

bool hasSupportedModel(const QString &model)
{
    return model.contains(QStringLiteral("DP570"), Qt::CaseInsensitive)
        || model.contains(QStringLiteral("DM32"), Qt::CaseInsensitive)
        || model.contains(QStringLiteral("DM-32"), Qt::CaseInsensitive);
}

bool performHandshake(WinSerialPort &port, QString &model, QString &error)
{
    QThread::msleep(DM32Constants::InitDelayMs);
    if (!port.clear()) {
        error = port.errorString();
        return false;
    }
    QThread::msleep(DM32Constants::ClearBufferDelayMs);

    QString ioError;
    if (!port.writeAll(QByteArrayLiteral("PSEARCH"), DM32Constants::RequestTimeoutMs, ioError)) {
        error = ioError;
        return false;
    }

    QThread::msleep(DM32Constants::PsearchReadDelayMs);
    const QByteArray psearch = port.readExact(8, DM32Constants::RequestTimeoutMs);
    if (psearch.size() != 8) {
        error = shortReadError(QStringLiteral("PSEARCH"), 8, psearch, port);
        return false;
    }
    if (byteValue(psearch.at(0)) != 0x06) {
        error = QStringLiteral("PSEARCH rejected: expected ACK 06, received [%1]").arg(toHex(psearch));
        return false;
    }

    model = QString::fromLatin1(psearch.mid(1));
    model.remove(QChar::Null);
    model = model.trimmed();
    if (!hasSupportedModel(model)) {
        error = QStringLiteral("Unsupported radio response: %1 [%2]").arg(model, toHex(psearch));
        return false;
    }

    QThread::msleep(50);
    if (!port.writeAll(QByteArrayLiteral("PASSSTA"), DM32Constants::RequestTimeoutMs, ioError)) {
        error = ioError;
        return false;
    }
    QThread::msleep(50);

    const QByteArray passsta = port.readExact(3, DM32Constants::RequestTimeoutMs);
    if (passsta.size() != 3) {
        error = shortReadError(QStringLiteral("PASSSTA"), 3, passsta, port);
        return false;
    }
    if (byteValue(passsta.at(0)) != 0x50) {
        error = QStringLiteral("PASSSTA failed: [%1]").arg(toHex(passsta));
        return false;
    }

    QThread::msleep(50);
    if (!port.writeAll(QByteArrayLiteral("SYSINFO"), DM32Constants::RequestTimeoutMs, ioError)) {
        error = ioError;
        return false;
    }
    QThread::msleep(50);

    const QByteArray sysinfo = port.readExact(1, DM32Constants::RequestTimeoutMs);
    if (sysinfo.size() != 1) {
        error = shortReadError(QStringLiteral("SYSINFO"), 1, sysinfo, port);
        return false;
    }
    if (byteValue(sysinfo.at(0)) != 0x06) {
        error = QStringLiteral("SYSINFO failed: [%1]").arg(toHex(sysinfo));
        return false;
    }

    QThread::msleep(10);
    return true;
}

bool queryVFrame(WinSerialPort &port, quint8 frameId, QByteArray &data, QString &error)
{
    QByteArray command;
    command.reserve(5);
    command.append(static_cast<char>(0x56));
    command.append(static_cast<char>(0x00));
    command.append(static_cast<char>(0x00));
    command.append(static_cast<char>(0x00));
    command.append(static_cast<char>(frameId));

    QString ioError;
    if (!port.writeAll(command, DM32Constants::RequestTimeoutMs, ioError)) {
        error = QStringLiteral("V-frame %1 write failed: %2").arg(hexByte(frameId), ioError);
        return false;
    }

    QThread::msleep(50);
    const QByteArray header = port.readExact(3, DM32Constants::RequestTimeoutMs);
    if (header.size() != 3) {
        error = shortReadError(QStringLiteral("V-frame %1 header").arg(hexByte(frameId)), 3, header, port);
        return false;
    }
    if (byteValue(header.at(0)) != 0x56 || byteValue(header.at(1)) != frameId) {
        error = QStringLiteral("Invalid V-frame %1 header [%2]").arg(hexByte(frameId), toHex(header));
        return false;
    }

    const quint8 length = byteValue(header.at(2));
    if (length == 0) {
        data.clear();
        QThread::msleep(50);
        return true;
    }

    data = port.readExact(length, DM32Constants::RequestTimeoutMs);
    if (data.size() != length) {
        error = shortReadError(QStringLiteral("V-frame %1 data").arg(hexByte(frameId)), length, data, port);
        return false;
    }

    QThread::msleep(50);
    return true;
}

QString parseAsciiFrame(const QByteArray &data)
{
    QString value = QString::fromLatin1(data);
    value.remove(QChar::Null);
    return value.trimmed();
}

quint32 readUint32LE(const QByteArray &data, int offset)
{
    return static_cast<quint32>(byteValue(data.at(offset)))
        | (static_cast<quint32>(byteValue(data.at(offset + 1))) << 8)
        | (static_cast<quint32>(byteValue(data.at(offset + 2))) << 16)
        | (static_cast<quint32>(byteValue(data.at(offset + 3))) << 24);
}

bool validateMemoryRange(quint32 startAddr, quint32 endAddr, int &blockCount, QString &error)
{
    if (startAddr == 0 || endAddr <= startAddr || endAddr > 0x00FFFFFF) {
        error = QStringLiteral("Radio returned an invalid config range %1-%2")
                    .arg(hexAddress(startAddr), hexAddress(endAddr));
        return false;
    }

    if ((startAddr & 0x0FFFU) != 0 || (endAddr & 0x0FFFU) != 0x0FFFU) {
        error = QStringLiteral("Config range is not 4KB aligned: %1-%2")
                    .arg(hexAddress(startAddr), hexAddress(endAddr));
        return false;
    }

    const quint64 byteCount = static_cast<quint64>(endAddr) - startAddr + 1U;
    if (byteCount == 0 || byteCount % static_cast<quint64>(DM32Constants::BlockSize) != 0) {
        error = QStringLiteral("Config range has an invalid byte length: %1").arg(byteCount);
        return false;
    }

    const quint64 blocks = byteCount / static_cast<quint64>(DM32Constants::BlockSize);
    if (blocks == 0 || blocks > 1024) {
        error = QStringLiteral("Refusing suspicious config range containing %1 blocks").arg(blocks);
        return false;
    }

    blockCount = static_cast<int>(blocks);
    return true;
}

bool enterProgrammingMode(WinSerialPort &port, QString &error)
{
    QByteArray programCommand;
    programCommand.reserve(12);
    programCommand.append(static_cast<char>(0xFF));
    programCommand.append(static_cast<char>(0xFF));
    programCommand.append(static_cast<char>(0xFF));
    programCommand.append(static_cast<char>(0xFF));
    programCommand.append(static_cast<char>(0x0C));
    programCommand.append(QByteArrayLiteral("PROGRAM"));

    QString ioError;
    if (!port.writeAll(programCommand, DM32Constants::RequestTimeoutMs, ioError)) {
        error = QStringLiteral("PROGRAM command write failed: %1").arg(ioError);
        return false;
    }

    const QByteArray ack1 = port.readExact(1, DM32Constants::RequestTimeoutMs);
    if (ack1.size() != 1 || byteValue(ack1.at(0)) != 0x06) {
        error = ack1.size() == 1
            ? QStringLiteral("PROGRAM command failed: expected ACK 06, received [%1]").arg(toHex(ack1))
            : shortReadError(QStringLiteral("PROGRAM ACK"), 1, ack1, port);
        return false;
    }
    QThread::msleep(10);

    const QByteArray modeCommand(1, static_cast<char>(0x02));
    if (!port.writeAll(modeCommand, DM32Constants::RequestTimeoutMs, ioError)) {
        error = QStringLiteral("PROGRAM mode 02 write failed: %1").arg(ioError);
        return false;
    }

    const QByteArray modeResponse = port.readExact(8, DM32Constants::RequestTimeoutMs);
    if (modeResponse.size() != 8) {
        error = shortReadError(QStringLiteral("PROGRAM mode 02"), 8, modeResponse, port);
        return false;
    }
    for (const char byte : modeResponse) {
        if (byteValue(byte) != 0xFF) {
            error = QStringLiteral("PROGRAM mode 02 failed: expected 8 x FF, received [%1]")
                        .arg(toHex(modeResponse));
            return false;
        }
    }
    QThread::msleep(10);

    const QByteArray finalAck(1, static_cast<char>(0x06));
    if (!port.writeAll(finalAck, DM32Constants::RequestTimeoutMs, ioError)) {
        error = QStringLiteral("PROGRAM final ACK write failed: %1").arg(ioError);
        return false;
    }

    const QByteArray ack2 = port.readExact(1, DM32Constants::RequestTimeoutMs);
    if (ack2.size() != 1 || byteValue(ack2.at(0)) != 0x06) {
        error = ack2.size() == 1
            ? QStringLiteral("PROGRAM final ACK failed: expected 06, received [%1]").arg(toHex(ack2))
            : shortReadError(QStringLiteral("PROGRAM final ACK"), 1, ack2, port);
        return false;
    }

    QThread::msleep(10);
    return true;
}

bool readMemory(WinSerialPort &port, quint32 address, quint16 length, QByteArray &data, QString &error)
{
    QByteArray command;
    command.reserve(6);
    command.append(static_cast<char>(0x52));
    command.append(static_cast<char>(address & 0xFFU));
    command.append(static_cast<char>((address >> 8) & 0xFFU));
    command.append(static_cast<char>((address >> 16) & 0xFFU));
    command.append(static_cast<char>(length & 0xFFU));
    command.append(static_cast<char>((length >> 8) & 0xFFU));

    QString ioError;
    if (!port.writeAll(command, DM32Constants::RequestTimeoutMs, ioError)) {
        error = QStringLiteral("Read command failed at %1: %2").arg(hexAddress(address), ioError);
        return false;
    }

    QThread::msleep(25);
    const QByteArray header = port.readExact(6, DM32Constants::RequestTimeoutMs);
    if (header.size() != 6) {
        error = shortReadError(QStringLiteral("Read header at %1").arg(hexAddress(address)), 6, header, port);
        return false;
    }
    if (byteValue(header.at(0)) != 0x57) {
        error = QStringLiteral("Invalid read response at %1: expected 57 header, received [%2]")
                    .arg(hexAddress(address), toHex(header));
        return false;
    }

    const quint16 responseLength = static_cast<quint16>(byteValue(header.at(4)))
        | (static_cast<quint16>(byteValue(header.at(5))) << 8);
    if (responseLength == 0 || responseLength > length) {
        error = QStringLiteral("Invalid read length at %1: requested %2, radio returned %3")
                    .arg(hexAddress(address))
                    .arg(length)
                    .arg(responseLength);
        return false;
    }

    data = port.readExact(responseLength, DM32Constants::ReadMemoryTimeoutMs);
    if (data.size() != responseLength) {
        error = shortReadError(QStringLiteral("Read data at %1").arg(hexAddress(address)), responseLength, data, port);
        return false;
    }

    QThread::msleep(30);
    return true;
}

QVector<int> referencedContactIds(int channelCount, const QVector<DM32MemoryBlock> &blocks)
{
    const QByteArray *low = nullptr;
    const QByteArray *high = nullptr;
    for (const DM32MemoryBlock &block : blocks) {
        if (block.metadata == TxContactLowMetadata) {
            low = &block.data;
        } else if (block.metadata == TxContactHighMetadata) {
            high = &block.data;
        }
    }

    QSet<int> ids;
    for (int channelNumber = 1; channelNumber <= channelCount; ++channelNumber) {
        const QByteArray *source = nullptr;
        qsizetype offset = 0;
        if (channelNumber <= 2047) {
            source = low;
            offset = static_cast<qsizetype>(channelNumber - 1) * 2;
        } else {
            source = high;
            offset = static_cast<qsizetype>(channelNumber & 0x7FF) * 2;
        }

        if (!source || offset < 0 || offset + 1 >= source->size()) {
            continue;
        }

        const quint8 byte0 = byteValue(source->at(offset));
        const quint8 byte1 = byteValue(source->at(offset + 1));
        const int contactId = static_cast<int>((byte0 >> 4) & 0x0F) * 256 + byte1;
        if (contactId > 0) {
            ids.insert(contactId);
        }
    }

    QVector<int> result;
    result.reserve(ids.size());
    for (const int id : ids) {
        result.push_back(id);
    }
    std::sort(result.begin(), result.end());
    return result;
}

const DM32MemoryBlock *findBlockByMetadata(
    const QVector<DM32MemoryBlock> &blocks,
    quint8 metadata)
{
    for (const DM32MemoryBlock &block : blocks) {
        if (block.metadata == metadata) {
            return &block;
        }
    }
    return nullptr;
}

struct DiscoveredBlock
{
    quint32 address {0};
    quint8 metadata {0};
};
} // namespace

DM32SelectiveReadResult DM32Connection::readChannelsSelective(
    const QString &portName,
    const ProgressCallback &onProgress)
{
    DM32SelectiveReadResult result;
    result.portName = portName;

    QElapsedTimer timer;
    timer.start();

    const auto finishElapsed = [&result, &timer] {
        result.elapsedMs = timer.elapsed();
    };
    const auto progress = [&onProgress](int value, const QString &message) {
        if (onProgress) {
            onProgress(qBound(0, value, 100), message);
        }
    };

    progress(0, QStringLiteral("READ RADIO // OPENING %1").arg(portName));

    WinSerialPort port;
    if (!port.open(portName, DM32Constants::BaudRate)) {
        result.error = port.errorString();
        finishElapsed();
        return result;
    }

    progress(2, QStringLiteral("READ RADIO // PSEARCH / PASSSTA / SYSINFO"));
    if (!performHandshake(port, result.model, result.error)) {
        finishElapsed();
        return result;
    }

    progress(5, QStringLiteral("READ RADIO // QUERY FIRMWARE + MEMORY MAP"));
    QByteArray firmwareFrame;
    if (!queryVFrame(port, 0x01, firmwareFrame, result.error)) {
        finishElapsed();
        return result;
    }
    result.firmware = parseAsciiFrame(firmwareFrame);

    QByteArray memoryFrame;
    if (!queryVFrame(port, 0x0A, memoryFrame, result.error)) {
        finishElapsed();
        return result;
    }
    if (memoryFrame.size() < 8) {
        result.error = QStringLiteral("Memory layout V-frame 0x0A returned %1 bytes; expected at least 8")
                           .arg(memoryFrame.size());
        finishElapsed();
        return result;
    }

    result.configStart = readUint32LE(memoryFrame, 0);
    result.configEnd = readUint32LE(memoryFrame, 4);
    int configBlockCount = 0;
    if (!validateMemoryRange(result.configStart, result.configEnd, configBlockCount, result.error)) {
        finishElapsed();
        return result;
    }

    // Channel TX-contact indexes refer to the normal codeplug Talk Groups table
    // (metadata 0x44), not the large external digital-ID database advertised by
    // V-frames 0x0F/0x10. Keep the whole Phase 5 read inside the proven config
    // region and fetch 0x44 plus its counter block 0x06 below.
    progress(7, QStringLiteral("READ RADIO // PREPARING CODEPLUG TALK GROUP TABLE"));

    progress(9, QStringLiteral("READ RADIO // ENTERING PROGRAM MODE // READ ONLY"));
    if (!enterProgrammingMode(port, result.error)) {
        finishElapsed();
        return result;
    }

    QVector<DiscoveredBlock> discovered;
    discovered.reserve(configBlockCount);
    QHash<int, quint32> metadataAddresses;

    progress(10, QStringLiteral("READ RADIO // DISCOVERING %1 BLOCK METADATA BYTES").arg(configBlockCount));
    for (int index = 0; index < configBlockCount; ++index) {
        const quint32 blockAddress = result.configStart
            + static_cast<quint32>(index * DM32Constants::BlockSize);
        const quint32 metadataAddress = blockAddress
            + static_cast<quint32>(DM32Constants::BlockSize - 1);

        QByteArray metadataData;
        if (!readMemory(port, metadataAddress, 1, metadataData, result.error)) {
            finishElapsed();
            return result;
        }
        if (metadataData.size() != 1) {
            result.error = QStringLiteral("Metadata scan returned %1 bytes at %2; expected 1")
                               .arg(metadataData.size())
                               .arg(hexAddress(metadataAddress));
            finishElapsed();
            return result;
        }

        const quint8 metadata = byteValue(metadataData.at(0));
        discovered.push_back({blockAddress, metadata});
        ++result.bytesTransferred;

        if (!metadataAddresses.contains(metadata)) {
            metadataAddresses.insert(metadata, blockAddress);
        }

        if ((index + 1) % 10 == 0 || index + 1 == configBlockCount) {
            const int percent = 10 + static_cast<int>(
                (static_cast<double>(index + 1) / configBlockCount) * 56.0);
            progress(
                percent,
                QStringLiteral("READ RADIO // MAP %1/%2 // %3 // META %4")
                    .arg(index + 1)
                    .arg(configBlockCount)
                    .arg(hexAddress(blockAddress))
                    .arg(hexByte(metadata)));
        }

        if (index + 1 < configBlockCount) {
            QThread::msleep(5);
        }
    }

    result.discoveredBlockCount = static_cast<int>(discovered.size());

    if (!metadataAddresses.contains(ChannelFirstMetadata)) {
        result.error = QStringLiteral("Metadata discovery did not find first channel block 0x12");
        finishElapsed();
        return result;
    }

    progress(68, QStringLiteral("READ RADIO // READING CHANNEL COUNT"));
    QByteArray countData;
    const quint32 firstChannelAddress = metadataAddresses.value(ChannelFirstMetadata);
    if (!readMemory(port, firstChannelAddress, 2, countData, result.error)) {
        finishElapsed();
        return result;
    }
    if (countData.size() != 2) {
        result.error = QStringLiteral("Channel count read returned %1 bytes; expected 2").arg(countData.size());
        finishElapsed();
        return result;
    }
    result.bytesTransferred += countData.size();
    result.channelCount = byteValue(countData.at(0))
        | (static_cast<int>(byteValue(countData.at(1))) << 8);

    if (result.channelCount < 0 || result.channelCount > MaximumChannels) {
        result.error = QStringLiteral("Radio returned invalid channel count %1").arg(result.channelCount);
        finishElapsed();
        return result;
    }

    const int remainingChannels = qMax(0, result.channelCount - ChannelsInFirstBlock);
    const int requiredChannelBlocks = result.channelCount == 0
        ? 1
        : 1 + ((remainingChannels + ChannelsInFollowingBlock - 1) / ChannelsInFollowingBlock);

    QVector<quint8> targetMetadata;
    targetMetadata.reserve(requiredChannelBlocks + 15);
    for (int index = 0; index < requiredChannelBlocks; ++index) {
        const quint8 metadata = static_cast<quint8>(ChannelFirstMetadata + index);
        if (metadata > ChannelLastMetadata || !metadataAddresses.contains(metadata)) {
            result.error = QStringLiteral("Channel map is incomplete: required metadata %1 was not discovered")
                               .arg(hexByte(metadata));
            finishElapsed();
            return result;
        }
        targetMetadata.push_back(metadata);
    }

    if (metadataAddresses.contains(TxContactLowMetadata)) {
        targetMetadata.push_back(TxContactLowMetadata);
    }
    if (result.channelCount >= 2048 && metadataAddresses.contains(TxContactHighMetadata)) {
        targetMetadata.push_back(TxContactHighMetadata);
    }
    if (metadataAddresses.contains(TalkGroupsMetadata)) {
        targetMetadata.push_back(TalkGroupsMetadata);
    }
    if (metadataAddresses.contains(TalkGroupCounterMetadata)) {
        targetMetadata.push_back(TalkGroupCounterMetadata);
    }
    if (metadataAddresses.contains(ScanListMetadata)) {
        targetMetadata.push_back(ScanListMetadata);
    }
    if (metadataAddresses.contains(RxGroupsMetadata)) {
        targetMetadata.push_back(RxGroupsMetadata);
    }
    for (int metadata = ZoneFirstMetadata; metadata <= ZoneLastMetadata; ++metadata) {
        if (metadataAddresses.contains(metadata)) {
            targetMetadata.push_back(static_cast<quint8>(metadata));
        }
    }

    progress(
        71,
        QStringLiteral("READ RADIO // %1 CHANNELS // FETCHING %2 CODEPLUG BLOCKS")
            .arg(result.channelCount)
            .arg(targetMetadata.size()));

    result.blocks.reserve(targetMetadata.size());
    for (qsizetype index = 0; index < targetMetadata.size(); ++index) {
        const quint8 metadata = targetMetadata.at(index);
        const quint32 address = metadataAddresses.value(metadata);

        QByteArray blockData;
        if (!readMemory(
                port,
                address,
                static_cast<quint16>(DM32Constants::BlockSize),
                blockData,
                result.error)) {
            finishElapsed();
            return result;
        }
        if (blockData.size() != DM32Constants::BlockSize) {
            result.error = QStringLiteral("Selective block %1 at %2 returned %3/%4 bytes")
                               .arg(hexByte(metadata))
                               .arg(hexAddress(address))
                               .arg(blockData.size())
                               .arg(DM32Constants::BlockSize);
            finishElapsed();
            return result;
        }

        const quint8 embeddedMetadata = byteValue(blockData.at(DM32Constants::BlockSize - 1));
        if (embeddedMetadata != metadata) {
            result.error = QStringLiteral("Selective block changed during read: expected metadata %1 at %2, received %3")
                               .arg(hexByte(metadata), hexAddress(address), hexByte(embeddedMetadata));
            finishElapsed();
            return result;
        }

        result.blocks.push_back({address, metadata, blockData});
        result.bytesTransferred += blockData.size();

        const int percent = 72 + static_cast<int>(
            (static_cast<double>(index + 1) / static_cast<double>(targetMetadata.size())) * 16.0);
        progress(
            percent,
            QStringLiteral("READ RADIO // CODEPLUG %1/%2 // %3 // META %4")
                .arg(index + 1)
                .arg(targetMetadata.size())
                .arg(hexAddress(address))
                .arg(hexByte(metadata)));
    }

    result.dataBlockCount = static_cast<int>(result.blocks.size());
    result.referencedContactIds = referencedContactIds(result.channelCount, result.blocks);

    if (!result.referencedContactIds.isEmpty()) {
        progress(
            89,
            QStringLiteral("READ RADIO // %1 REFERENCED CONTACTS // RESOLVING CODEPLUG TALK GROUPS")
                .arg(result.referencedContactIds.size()));

        const DM32MemoryBlock *talkGroups = findBlockByMetadata(result.blocks, TalkGroupsMetadata);
        if (talkGroups) {
            result.contactBlocks.push_back({talkGroups->address, 0, talkGroups->data});
            result.contactBlockCount = 1;

            const DM32MemoryBlock *counter = findBlockByMetadata(result.blocks, TalkGroupCounterMetadata);
            if (counter && counter->data.size() > TalkGroupCounterOffset) {
                result.contactDatabaseCount = byteValue(counter->data.at(TalkGroupCounterOffset));
            } else {
                result.contactWarning = QStringLiteral("TALK GROUP COUNTER BLOCK 0x06 NOT AVAILABLE");
            }
        } else {
            result.contactWarning = QStringLiteral("TALK GROUP DATA BLOCK 0x44 NOT AVAILABLE");
        }
    }

    progress(98, QStringLiteral("READ RADIO // CLOSING RADIO SESSION"));
    port.close();

    result.ok = true;
    finishElapsed();
    progress(
        100,
        QStringLiteral("READ RADIO COMPLETE // %1 CHANNELS // %2 CONTACTS REF // %3 BYTES // %4 ms")
            .arg(result.channelCount)
            .arg(result.referencedContactIds.size())
            .arg(result.bytesTransferred)
            .arg(result.elapsedMs));
    return result;
}
