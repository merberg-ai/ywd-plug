#include "AppController.h"

#include "radios/dm32uv/DM32ChannelDecoder.h"
#include "radios/dm32uv/DM32CodeplugDecoder.h"
#include "serial/WinSerialPort.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QPointer>
#include <QStandardPaths>
#include <QStringList>
#include <QVariantMap>
#include <QtConcurrent/QtConcurrentRun>

namespace {
QString joinedIntegers(const QVector<int> &values)
{
    QStringList text;
    text.reserve(values.size());
    for (const int value : values) {
        text.push_back(QString::number(value));
    }
    return text.join(QStringLiteral(", "));
}

QString joinedIds(const QVector<quint32> &values)
{
    QStringList text;
    text.reserve(values.size());
    for (const quint32 value : values) {
        text.push_back(QString::number(value));
    }
    return text.join(QStringLiteral(", "));
}

QVariantList integerList(const QVector<int> &values)
{
    QVariantList result;
    result.reserve(values.size());
    for (const int value : values) {
        result.push_back(value);
    }
    return result;
}

QVariantList idList(const QVector<quint32> &values)
{
    QVariantList result;
    result.reserve(values.size());
    for (const quint32 value : values) {
        result.push_back(QVariant::fromValue<qulonglong>(value));
    }
    return result;
}

QString channelSelectorText(int type, int channel)
{
    if (type <= 0) {
        return QStringLiteral("NONE");
    }
    if (type == 1) {
        return QStringLiteral("CURRENT");
    }
    if (channel > 0) {
        return QStringLiteral("CH %1").arg(channel);
    }
    return QStringLiteral("TYPE %1").arg(type);
}
} // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    connect(&m_probeWatcher, &QFutureWatcher<DM32ProbeResult>::finished, this, [this] {
        const auto result = m_probeWatcher.result();
        setBusy(false);
        setOperation(QString());

        if (result.ok) {
            m_radioDetected = true;
            m_radioModel = result.model;
            m_detectedPort = result.portName;
            setStatus(QStringLiteral("RADIO DETECTED // %1 // %2").arg(result.model, result.portName));
            emit radioChanged();
            return;
        }

        m_radioDetected = false;
        m_radioModel.clear();
        m_detectedPort.clear();
        setStatus(QStringLiteral("PROBE FAILED // %1").arg(result.error));
        emit radioChanged();
    });

    connect(&m_selectiveReadWatcher, &QFutureWatcher<DM32SelectiveReadResult>::finished, this, [this] {
        const auto result = m_selectiveReadWatcher.result();
        setBusy(false);
        setOperation(QString());

        if (!result.ok) {
            m_liveReadReady = false;
            setStatus(QStringLiteral("READ RADIO FAILED // %1").arg(result.error));
            emit readStatsChanged();
            return;
        }

        const auto decodedChannels = DM32ChannelDecoder::decodeBlocks(result.blocks);
        QString decodeError;
        if (!applyDecodedChannels(decodedChannels, decodeError)) {
            m_liveReadReady = false;
            setStatus(QStringLiteral("READ RADIO CHANNEL DECODE FAILED // %1").arg(decodeError));
            emit readStatsChanged();
            return;
        }

        const auto decodedCodeplug = DM32CodeplugDecoder::decodeBlocks(result.blocks);
        if (!applyDecodedCodeplug(decodedCodeplug, decodeError)) {
            m_liveReadReady = false;
            setStatus(QStringLiteral("READ RADIO STRUCTURE DECODE FAILED // %1").arg(decodeError));
            emit readStatsChanged();
            return;
        }

        m_radioDetected = true;
        m_radioModel = result.model;
        m_detectedPort = result.portName;
        m_liveReadReady = true;
        m_lastReadBytes = result.bytesTransferred;
        m_lastReadMs = result.elapsedMs;
        m_lastReadDataBlocks = result.dataBlockCount;
        m_lastReadDiscoveredBlocks = result.discoveredBlockCount;
        setReadProgress(100);

        setStatus(
            QStringLiteral("READ RADIO COMPLETE // %1 CH // %2 ZONES // %3 SCAN // %4 RXG // %5 BYTES // %6s")
                .arg(m_channelCount)
                .arg(m_zones.size())
                .arg(m_scanLists.size())
                .arg(m_rxGroups.size())
                .arg(result.bytesTransferred)
                .arg(QString::number(static_cast<double>(result.elapsedMs) / 1000.0, 'f', 1)));

        emit radioChanged();
        emit readStatsChanged();
    });

    connect(&m_backupWatcher, &QFutureWatcher<DM32BackupResult>::finished, this, [this] {
        const auto result = m_backupWatcher.result();
        setBusy(false);
        setOperation(QString());

        if (!result.ok) {
            m_backupReady = false;
            clearChannelState();
            clearCodeplugState();
            setStatus(QStringLiteral("RAW BACKUP FAILED // %1").arg(result.error));
            emit backupChanged();
            return;
        }

        m_radioDetected = true;
        m_radioModel = result.model;
        m_detectedPort = result.portName;
        m_backupReady = true;
        m_backupPath = result.backupPath;
        m_backupManifestPath = result.manifestPath;
        m_backupSha256 = result.sha256;
        setReadProgress(100);

        QString decodeError;
        if (loadCodeplugFromBackup(result.backupPath, decodeError)) {
            setStatus(
                QStringLiteral("RAW BACKUP COMPLETE // %1 CH // %2 ZONES // %3 SCAN // %4 RXG // SHA256 %5")
                    .arg(m_channelCount)
                    .arg(m_zones.size())
                    .arg(m_scanLists.size())
                    .arg(m_rxGroups.size())
                    .arg(result.sha256.left(16).toUpper()));
        } else {
            setStatus(QStringLiteral("RAW BACKUP COMPLETE // DECODE FAILED // %1").arg(decodeError));
        }

        emit radioChanged();
        emit backupChanged();
    });

    refreshPorts();
}

void AppController::refreshPorts()
{
    QVariantList nextPorts;

    for (const QString &portName : WinSerialPort::availablePorts()) {
        QVariantMap item;
        item.insert(QStringLiteral("name"), portName);
        item.insert(QStringLiteral("description"), QStringLiteral("Windows COM port"));
        item.insert(QStringLiteral("manufacturer"), QString());
        item.insert(QStringLiteral("serialNumber"), QString());
        item.insert(QStringLiteral("vendorId"), QStringLiteral("----"));
        item.insert(QStringLiteral("productId"), QStringLiteral("----"));
        item.insert(QStringLiteral("label"), QStringLiteral("%1 — Windows COM port").arg(portName));
        nextPorts.push_back(item);
    }

    m_ports = nextPorts;
    emit portsChanged();

    if (m_ports.isEmpty()) {
        setStatus(QStringLiteral("NO SERIAL PORTS FOUND"));
    } else if (!m_busy && !m_radioDetected) {
        setStatus(QStringLiteral("READY // %1 SERIAL PORT%2")
                      .arg(m_ports.size())
                      .arg(m_ports.size() == 1 ? QString() : QStringLiteral("S")));
    }
}

void AppController::probePort(const QString &portName)
{
    if (m_busy) {
        return;
    }

    if (portName.trimmed().isEmpty()) {
        setStatus(QStringLiteral("SELECT A COM PORT FIRST"));
        return;
    }

    m_radioDetected = false;
    m_radioModel.clear();
    m_detectedPort.clear();
    clearBackupState();
    clearReadStats();
    emit radioChanged();

    setReadProgress(0);
    setOperation(QStringLiteral("probe"));
    setBusy(true);
    setStatus(QStringLiteral("PROBING %1 // PSEARCH").arg(portName));

    m_probeWatcher.setFuture(QtConcurrent::run([portName] {
        return DM32Connection::probe(portName);
    }));
}

void AppController::readRadio(const QString &portName)
{
    if (m_busy) {
        return;
    }

    if (portName.trimmed().isEmpty()) {
        setStatus(QStringLiteral("SELECT A COM PORT FIRST"));
        return;
    }

    m_radioDetected = false;
    m_radioModel.clear();
    m_detectedPort.clear();
    clearBackupState();
    clearReadStats();
    emit radioChanged();

    setReadProgress(0);
    setOperation(QStringLiteral("read"));
    setBusy(true);
    setStatus(QStringLiteral("READ RADIO // PREPARING SELECTIVE CODEPLUG SESSION"));

    const QPointer<AppController> self(this);
    const DM32Connection::ProgressCallback progress = [self](int value, const QString &message) {
        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(
            self,
            [self, value, message] {
                if (!self) {
                    return;
                }
                self->setReadProgress(value);
                self->setStatus(message);
            },
            Qt::QueuedConnection);
    };

    m_selectiveReadWatcher.setFuture(QtConcurrent::run([portName, progress] {
        return DM32Connection::readChannelsSelective(portName, progress);
    }));
}

void AppController::readRawBackup(const QString &portName)
{
    if (m_busy) {
        return;
    }

    if (portName.trimmed().isEmpty()) {
        setStatus(QStringLiteral("SELECT A COM PORT FIRST"));
        return;
    }

    if (!m_radioDetected || m_detectedPort.compare(portName, Qt::CaseInsensitive) != 0) {
        setStatus(QStringLiteral("RAW BACKUP BLOCKED // PROBE OR READ THE SELECTED PORT FIRST"));
        return;
    }

    clearBackupState();
    clearReadStats();
    setReadProgress(0);
    setOperation(QStringLiteral("backup"));
    setBusy(true);
    setStatus(QStringLiteral("RAW BACKUP // PREPARING READ-ONLY SESSION"));

    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documents.isEmpty()) {
        documents = QDir::homePath();
    }
    const QString outputDirectory = QDir(documents).filePath(QStringLiteral("YWD-Plug Backups"));

    const QPointer<AppController> self(this);
    const DM32Connection::ProgressCallback progress = [self](int value, const QString &message) {
        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(
            self,
            [self, value, message] {
                if (!self) {
                    return;
                }
                self->setReadProgress(value);
                self->setStatus(message);
            },
            Qt::QueuedConnection);
    };

    m_backupWatcher.setFuture(QtConcurrent::run([portName, outputDirectory, progress] {
        return DM32Connection::readRawBackup(portName, outputDirectory, progress);
    }));
}

void AppController::loadLatestBackup()
{
    if (m_busy) {
        return;
    }

    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documents.isEmpty()) {
        documents = QDir::homePath();
    }

    QDir backupDirectory(QDir(documents).filePath(QStringLiteral("YWD-Plug Backups")));
    const QFileInfoList backups = backupDirectory.entryInfoList(
        {QStringLiteral("YWD-Plug-*.bin")},
        QDir::Files | QDir::Readable,
        QDir::Time);

    if (backups.isEmpty()) {
        setStatus(QStringLiteral("NO RAW BACKUPS FOUND // %1").arg(backupDirectory.absolutePath()));
        return;
    }

    const QFileInfo backupInfo = backups.first();
    clearBackupState();
    clearReadStats();

    QString decodeError;
    if (!loadCodeplugFromBackup(backupInfo.absoluteFilePath(), decodeError)) {
        setStatus(QStringLiteral("LOAD BACKUP FAILED // %1").arg(decodeError));
        return;
    }

    QFile backupFile(backupInfo.absoluteFilePath());
    if (backupFile.open(QIODevice::ReadOnly)) {
        m_backupSha256 = QString::fromLatin1(
            QCryptographicHash::hash(backupFile.readAll(), QCryptographicHash::Sha256).toHex());
    }

    const QString manifestCandidate = backupDirectory.filePath(
        backupInfo.completeBaseName() + QStringLiteral(".json"));

    m_backupReady = true;
    m_backupPath = backupInfo.absoluteFilePath();
    m_backupManifestPath = QFileInfo::exists(manifestCandidate) ? manifestCandidate : QString();
    setReadProgress(100);
    setStatus(
        QStringLiteral("OFFLINE BACKUP LOADED // %1 CH // %2 ZONES // %3 SCAN // %4 RXG // SHA256 %5")
            .arg(m_channelCount)
            .arg(m_zones.size())
            .arg(m_scanLists.size())
            .arg(m_rxGroups.size())
            .arg(m_backupSha256.left(16).toUpper()));
    emit backupChanged();
}

void AppController::clearRadio()
{
    if (m_busy) {
        return;
    }

    m_radioDetected = false;
    m_radioModel.clear();
    m_detectedPort.clear();
    clearBackupState();
    clearReadStats();
    setReadProgress(0);
    setOperation(QString());
    setStatus(QStringLiteral("READY // SELECT A COM PORT"));
    emit radioChanged();
}

void AppController::setStatus(const QString &status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;
    emit statusChanged();
}

void AppController::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }
    m_busy = busy;
    emit busyChanged();
}

void AppController::setReadProgress(int progress)
{
    const int bounded = qBound(0, progress, 100);
    if (m_readProgress == bounded) {
        return;
    }
    m_readProgress = bounded;
    emit readProgressChanged();
}

void AppController::setOperation(const QString &operation)
{
    if (m_operation == operation) {
        return;
    }
    m_operation = operation;
    emit operationChanged();
}

void AppController::clearBackupState()
{
    const bool changed = m_backupReady
        || !m_backupPath.isEmpty()
        || !m_backupManifestPath.isEmpty()
        || !m_backupSha256.isEmpty();

    m_backupReady = false;
    m_backupPath.clear();
    m_backupManifestPath.clear();
    m_backupSha256.clear();
    clearChannelState();
    clearCodeplugState();

    if (changed) {
        emit backupChanged();
    }
}

void AppController::clearChannelState()
{
    const bool changed = m_channelsReady || m_channelCount != 0 || !m_channels.isEmpty();

    m_channels.clear();
    m_channelCount = 0;
    m_channelsReady = false;

    if (changed) {
        emit channelsChanged();
    }
}

void AppController::clearCodeplugState()
{
    const bool changed = m_codeplugReady
        || !m_zones.isEmpty()
        || !m_scanLists.isEmpty()
        || !m_rxGroups.isEmpty();

    m_zones.clear();
    m_scanLists.clear();
    m_rxGroups.clear();
    m_codeplugReady = false;

    if (changed) {
        emit codeplugChanged();
    }
}

void AppController::clearReadStats()
{
    const bool changed = m_liveReadReady
        || m_lastReadBytes != 0
        || m_lastReadMs != 0
        || m_lastReadDataBlocks != 0
        || m_lastReadDiscoveredBlocks != 0;

    m_liveReadReady = false;
    m_lastReadBytes = 0;
    m_lastReadMs = 0;
    m_lastReadDataBlocks = 0;
    m_lastReadDiscoveredBlocks = 0;

    if (changed) {
        emit readStatsChanged();
    }
}

bool AppController::applyDecodedChannels(const DM32ChannelDecodeResult &decoded, QString &error)
{
    error.clear();
    if (!decoded.ok) {
        clearChannelState();
        error = decoded.error;
        return false;
    }

    QVariantList channels;
    channels.reserve(decoded.channels.size());

    for (const DM32ChannelInfo &channel : decoded.channels) {
        QVariantMap item;
        item.insert(QStringLiteral("number"), channel.number);
        item.insert(QStringLiteral("name"), channel.name);
        item.insert(QStringLiteral("rxFrequency"), channel.rxFrequency);
        item.insert(QStringLiteral("txFrequency"), channel.txFrequency);
        item.insert(QStringLiteral("txDisabled"), channel.txDisabled);
        item.insert(QStringLiteral("mode"), channel.mode);
        item.insert(QStringLiteral("power"), channel.power);
        item.insert(QStringLiteral("bandwidth"), channel.bandwidth);
        item.insert(QStringLiteral("scanAdd"), channel.scanAdd);
        item.insert(QStringLiteral("scanListId"), channel.scanListId);
        item.insert(QStringLiteral("colorCode"), channel.colorCode);
        item.insert(QStringLiteral("timeSlot"), channel.timeSlot);
        item.insert(QStringLiteral("rxGroupListId"), channel.rxGroupListId);
        item.insert(QStringLiteral("txContactIndex"), channel.txContactIndex);
        item.insert(QStringLiteral("txContactDigital"), channel.txContactDigital);
        channels.push_back(item);
    }

    m_channels = channels;
    m_channelCount = decoded.channelCount;
    m_channelsReady = true;
    emit channelsChanged();
    return true;
}

bool AppController::applyDecodedCodeplug(const DM32CodeplugDecodeResult &decoded, QString &error)
{
    error.clear();
    if (!decoded.ok) {
        clearCodeplugState();
        error = decoded.error;
        return false;
    }

    QVariantList zones;
    zones.reserve(decoded.zones.size());
    for (const DM32ZoneInfo &zone : decoded.zones) {
        QVariantMap item;
        item.insert(QStringLiteral("number"), zone.number);
        item.insert(QStringLiteral("name"), zone.name);
        item.insert(QStringLiteral("channelCount"), zone.channels.size());
        item.insert(QStringLiteral("channels"), integerList(zone.channels));
        item.insert(QStringLiteral("channelsText"), joinedIntegers(zone.channels));
        zones.push_back(item);
    }

    QVariantList scanLists;
    scanLists.reserve(decoded.scanLists.size());
    for (const DM32ScanListInfo &scan : decoded.scanLists) {
        QVariantMap item;
        item.insert(QStringLiteral("number"), scan.number);
        item.insert(QStringLiteral("name"), scan.name);
        item.insert(QStringLiteral("channelCount"), scan.channels.size());
        item.insert(QStringLiteral("channels"), integerList(scan.channels));
        item.insert(QStringLiteral("channelsText"), joinedIntegers(scan.channels));
        item.insert(QStringLiteral("hangTime"), QString::number(scan.hangTimeSeconds, 'f', 1) + QStringLiteral("s"));
        item.insert(QStringLiteral("priority1"), channelSelectorText(scan.priority1Type, scan.priority1Channel));
        item.insert(QStringLiteral("priority2"), channelSelectorText(scan.priority2Type, scan.priority2Channel));
        item.insert(QStringLiteral("designatedTx"), channelSelectorText(scan.designatedTxMode, scan.designatedTxChannel));
        scanLists.push_back(item);
    }

    QVariantList rxGroups;
    rxGroups.reserve(decoded.rxGroups.size());
    for (const DM32RxGroupInfo &group : decoded.rxGroups) {
        QVariantMap item;
        item.insert(QStringLiteral("number"), group.number);
        item.insert(QStringLiteral("name"), group.name);
        item.insert(QStringLiteral("memberCount"), group.memberIds.size());
        item.insert(QStringLiteral("members"), idList(group.memberIds));
        item.insert(QStringLiteral("membersText"), joinedIds(group.memberIds));
        rxGroups.push_back(item);
    }

    m_zones = zones;
    m_scanLists = scanLists;
    m_rxGroups = rxGroups;
    m_codeplugReady = true;
    emit codeplugChanged();
    return true;
}

bool AppController::loadCodeplugFromBackup(const QString &path, QString &error)
{
    error.clear();

    const auto channels = DM32ChannelDecoder::decodeFile(path);
    if (!applyDecodedChannels(channels, error)) {
        return false;
    }

    const auto codeplug = DM32CodeplugDecoder::decodeFile(path);
    if (!applyDecodedCodeplug(codeplug, error)) {
        clearChannelState();
        return false;
    }

    return true;
}
