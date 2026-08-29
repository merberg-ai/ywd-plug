#include "DM32Connection.h"

#include "DM32Constants.h"
#include "serial/WinSerialPort.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QThread>

#include <array>

namespace {
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

bool hasSupportedModel(const QString &model)
{
    return model.contains(QStringLiteral("DP570"), Qt::CaseInsensitive)
        || model.contains(QStringLiteral("DM32"), Qt::CaseInsensitive)
        || model.contains(QStringLiteral("DM-32"), Qt::CaseInsensitive);
}

QString shortReadError(const QString &stage, qsizetype expected, const QByteArray &received, const WinSerialPort &port)
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

bool performHandshake(WinSerialPort &port, QString &model, QByteArray *psearchResponse, QString &error)
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
    const auto psearch = port.readExact(8, DM32Constants::RequestTimeoutMs);
    if (psearchResponse) {
        *psearchResponse = psearch;
    }

    if (psearch.size() != 8) {
        error = shortReadError(QStringLiteral("PSEARCH"), 8, psearch, port);
        return false;
    }

    if (static_cast<quint8>(psearch.at(0)) != 0x06) {
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

    const auto passsta = port.readExact(3, DM32Constants::RequestTimeoutMs);
    if (passsta.size() != 3) {
        error = shortReadError(QStringLiteral("PASSSTA"), 3, passsta, port);
        return false;
    }
    if (static_cast<quint8>(passsta.at(0)) != 0x50) {
        error = QStringLiteral("PASSSTA failed: [%1]").arg(toHex(passsta));
        return false;
    }

    QThread::msleep(50);
    if (!port.writeAll(QByteArrayLiteral("SYSINFO"), DM32Constants::RequestTimeoutMs, ioError)) {
        error = ioError;
        return false;
    }
    QThread::msleep(50);

    const auto sysinfo = port.readExact(1, DM32Constants::RequestTimeoutMs);
    if (sysinfo.size() != 1) {
        error = shortReadError(QStringLiteral("SYSINFO"), 1, sysinfo, port);
        return false;
    }
    if (static_cast<quint8>(sysinfo.at(0)) != 0x06) {
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
    const auto header = port.readExact(3, DM32Constants::RequestTimeoutMs);
    if (header.size() != 3) {
        error = shortReadError(QStringLiteral("V-frame %1 header").arg(hexByte(frameId)), 3, header, port);
        return false;
    }

    if (static_cast<quint8>(header.at(0)) != 0x56
        || static_cast<quint8>(header.at(1)) != frameId) {
        error = QStringLiteral("Invalid V-frame %1 header [%2]").arg(hexByte(frameId), toHex(header));
        return false;
    }

    const auto length = static_cast<quint8>(header.at(2));
    if (length == 0) {
        data.clear();
        QThread::msleep(50);
        return true;
    }

    data = port.readExact(length, DM32Constants::RequestTimeoutMs);
    if (data.size() != length) {
        error = shortReadError(
            QStringLiteral("V-frame %1 data").arg(hexByte(frameId)),
            length,
            data,
            port);
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
    return static_cast<quint32>(static_cast<quint8>(data.at(offset)))
        | (static_cast<quint32>(static_cast<quint8>(data.at(offset + 1))) << 8)
        | (static_cast<quint32>(static_cast<quint8>(data.at(offset + 2))) << 16)
        | (static_cast<quint32>(static_cast<quint8>(data.at(offset + 3))) << 24);
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

    const auto ack1 = port.readExact(1, DM32Constants::RequestTimeoutMs);
    if (ack1.size() != 1 || static_cast<quint8>(ack1.at(0)) != 0x06) {
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

    const auto modeResponse = port.readExact(8, DM32Constants::RequestTimeoutMs);
    if (modeResponse.size() != 8) {
        error = shortReadError(QStringLiteral("PROGRAM mode 02"), 8, modeResponse, port);
        return false;
    }
    for (const char byte : modeResponse) {
        if (static_cast<quint8>(byte) != 0xFF) {
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

    const auto ack2 = port.readExact(1, DM32Constants::RequestTimeoutMs);
    if (ack2.size() != 1 || static_cast<quint8>(ack2.at(0)) != 0x06) {
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
    const auto header = port.readExact(6, DM32Constants::RequestTimeoutMs);
    if (header.size() != 6) {
        error = shortReadError(QStringLiteral("Read header at %1").arg(hexAddress(address)), 6, header, port);
        return false;
    }

    if (static_cast<quint8>(header.at(0)) != 0x57) {
        error = QStringLiteral("Invalid read response at %1: expected 57 header, received [%2]")
                    .arg(hexAddress(address), toHex(header));
        return false;
    }

    const quint16 responseLength = static_cast<quint16>(static_cast<quint8>(header.at(4)))
        | (static_cast<quint16>(static_cast<quint8>(header.at(5))) << 8);
    if (responseLength == 0 || responseLength > length) {
        error = QStringLiteral("Invalid read length at %1: requested %2, radio returned %3")
                    .arg(hexAddress(address))
                    .arg(length)
                    .arg(responseLength);
        return false;
    }

    data = port.readExact(responseLength, DM32Constants::ReadMemoryTimeoutMs);
    if (data.size() != responseLength) {
        error = shortReadError(
            QStringLiteral("Read data at %1").arg(hexAddress(address)),
            responseLength,
            data,
            port);
        return false;
    }

    QThread::msleep(30);
    return true;
}

bool writeBackupFiles(
    const DM32BackupResult &result,
    const QByteArray &image,
    const QJsonArray &blocks,
    const QString &outputDirectory,
    QString &backupPath,
    QString &manifestPath,
    QString &sha256,
    QString &error)
{
    QDir dir(outputDirectory);
    if (!dir.exists() && !QDir().mkpath(outputDirectory)) {
        error = QStringLiteral("Could not create backup directory: %1").arg(outputDirectory);
        return false;
    }

    QString safeModel = result.model;
    safeModel.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]+")), QStringLiteral("_"));
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const QString baseName = QStringLiteral("YWD-Plug-%1-%2").arg(safeModel, stamp);

    backupPath = dir.filePath(baseName + QStringLiteral(".bin"));
    manifestPath = dir.filePath(baseName + QStringLiteral(".json"));

    QSaveFile backupFile(backupPath);
    if (!backupFile.open(QIODevice::WriteOnly)) {
        error = QStringLiteral("Could not open raw backup for writing: %1").arg(backupFile.errorString());
        return false;
    }
    if (backupFile.write(image) != image.size() || !backupFile.commit()) {
        error = QStringLiteral("Could not commit raw backup: %1").arg(backupFile.errorString());
        return false;
    }

    sha256 = QString::fromLatin1(QCryptographicHash::hash(image, QCryptographicHash::Sha256).toHex());

    QJsonObject manifest;
    manifest.insert(QStringLiteral("format"), QStringLiteral("ywd-plug-native-raw-backup-v1"));
    manifest.insert(QStringLiteral("createdUtc"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    manifest.insert(QStringLiteral("model"), result.model);
    manifest.insert(QStringLiteral("firmware"), result.firmware);
    manifest.insert(QStringLiteral("port"), result.portName);
    manifest.insert(QStringLiteral("baudRate"), DM32Constants::BaudRate);
    manifest.insert(QStringLiteral("configStart"), hexAddress(result.configStart));
    manifest.insert(QStringLiteral("configEnd"), hexAddress(result.configEnd));
    manifest.insert(QStringLiteral("blockSize"), static_cast<int>(DM32Constants::BlockSize));
    manifest.insert(QStringLiteral("blockCount"), result.blockCount);
    manifest.insert(QStringLiteral("bytes"), static_cast<double>(image.size()));
    manifest.insert(QStringLiteral("sha256"), sha256);
    manifest.insert(QStringLiteral("writesPerformed"), false);
    manifest.insert(QStringLiteral("blocks"), blocks);

    QSaveFile manifestFile(manifestPath);
    if (!manifestFile.open(QIODevice::WriteOnly)) {
        error = QStringLiteral("Raw backup saved, but manifest open failed: %1").arg(manifestFile.errorString());
        return false;
    }

    const QByteArray json = QJsonDocument(manifest).toJson(QJsonDocument::Indented);
    if (manifestFile.write(json) != json.size() || !manifestFile.commit()) {
        error = QStringLiteral("Raw backup saved, but manifest commit failed: %1").arg(manifestFile.errorString());
        return false;
    }

    return true;
}
} // namespace

DM32ProbeResult DM32Connection::probe(const QString &portName)
{
    DM32ProbeResult result;
    result.portName = portName;

    WinSerialPort port;
    if (!port.open(portName, DM32Constants::BaudRate)) {
        result.error = port.errorString();
        return result;
    }

    if (!performHandshake(port, result.model, &result.psearchResponse, result.error)) {
        return result;
    }

    result.ok = true;
    return result;
}

DM32BackupResult DM32Connection::readRawBackup(
    const QString &portName,
    const QString &outputDirectory,
    const ProgressCallback &onProgress)
{
    DM32BackupResult result;
    result.portName = portName;

    const auto progress = [&onProgress](int value, const QString &message) {
        if (onProgress) {
            onProgress(qBound(0, value, 100), message);
        }
    };

    progress(0, QStringLiteral("RAW BACKUP // OPENING %1").arg(portName));

    WinSerialPort port;
    if (!port.open(portName, DM32Constants::BaudRate)) {
        result.error = port.errorString();
        return result;
    }

    progress(2, QStringLiteral("RAW BACKUP // PSEARCH / PASSSTA / SYSINFO"));
    if (!performHandshake(port, result.model, nullptr, result.error)) {
        return result;
    }

    progress(5, QStringLiteral("RAW BACKUP // QUERY FIRMWARE V-FRAME 0x01"));
    QByteArray firmwareFrame;
    if (!queryVFrame(port, 0x01, firmwareFrame, result.error)) {
        return result;
    }
    result.firmware = parseAsciiFrame(firmwareFrame);

    progress(7, QStringLiteral("RAW BACKUP // QUERY MEMORY LAYOUT V-FRAME 0x0A"));
    QByteArray memoryFrame;
    if (!queryVFrame(port, 0x0A, memoryFrame, result.error)) {
        return result;
    }
    if (memoryFrame.size() < 8) {
        result.error = QStringLiteral("Memory layout V-frame 0x0A returned %1 bytes; expected at least 8")
                           .arg(memoryFrame.size());
        return result;
    }

    result.configStart = readUint32LE(memoryFrame, 0);
    result.configEnd = readUint32LE(memoryFrame, 4);
    if (!validateMemoryRange(result.configStart, result.configEnd, result.blockCount, result.error)) {
        return result;
    }

    progress(
        9,
        QStringLiteral("RAW BACKUP // RANGE %1-%2 // %3 BLOCKS")
            .arg(hexAddress(result.configStart), hexAddress(result.configEnd))
            .arg(result.blockCount));

    if (!enterProgrammingMode(port, result.error)) {
        return result;
    }

    progress(10, QStringLiteral("RAW BACKUP // PROGRAM MODE ENTERED // READ ONLY"));

    QByteArray image;
    image.reserve(static_cast<qsizetype>(result.blockCount) * DM32Constants::BlockSize);
    QJsonArray blockManifest;

    for (int blockIndex = 0; blockIndex < result.blockCount; ++blockIndex) {
        const quint32 address = result.configStart
            + static_cast<quint32>(blockIndex * DM32Constants::BlockSize);

        QByteArray block;
        if (!readMemory(
                port,
                address,
                static_cast<quint16>(DM32Constants::BlockSize),
                block,
                result.error)) {
            return result;
        }

        if (block.size() != DM32Constants::BlockSize) {
            result.error = QStringLiteral("Short 4KB block at %1: received %2/%3 bytes")
                               .arg(hexAddress(address))
                               .arg(block.size())
                               .arg(DM32Constants::BlockSize);
            return result;
        }

        image.append(block);

        const quint8 metadata = static_cast<quint8>(block.at(DM32Constants::BlockSize - 1));
        QJsonObject blockEntry;
        blockEntry.insert(QStringLiteral("index"), blockIndex);
        blockEntry.insert(QStringLiteral("address"), hexAddress(address));
        blockEntry.insert(QStringLiteral("metadata"), hexByte(metadata));
        blockManifest.append(blockEntry);

        const int percent = 10 + static_cast<int>(
            (static_cast<double>(blockIndex + 1) / result.blockCount) * 84.0);
        progress(
            percent,
            QStringLiteral("RAW BACKUP // BLOCK %1/%2 // %3 // META %4")
                .arg(blockIndex + 1)
                .arg(result.blockCount)
                .arg(hexAddress(address))
                .arg(hexByte(metadata)));

        if (blockIndex + 1 < result.blockCount) {
            QThread::msleep(DM32Constants::BlockReadDelayMs);
        }
    }

    result.bytesRead = image.size();

    progress(95, QStringLiteral("RAW BACKUP // CLOSING RADIO SESSION"));
    port.close();

    progress(96, QStringLiteral("RAW BACKUP // HASHING %1 BYTES").arg(image.size()));
    if (!writeBackupFiles(
            result,
            image,
            blockManifest,
            outputDirectory,
            result.backupPath,
            result.manifestPath,
            result.sha256,
            result.error)) {
        return result;
    }

    result.ok = true;
    progress(100, QStringLiteral("RAW BACKUP COMPLETE // SHA256 %1").arg(result.sha256.left(16).toUpper()));
    return result;
}