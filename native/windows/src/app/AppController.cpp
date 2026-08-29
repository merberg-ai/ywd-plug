#include "AppController.h"

#include "radios/dm32uv/DM32ChannelDecoder.h"
#include "serial/WinSerialPort.h"

#include <QDir>
#include <QMetaObject>
#include <QPointer>
#include <QStandardPaths>
#include <QVariantMap>
#include <QtConcurrent/QtConcurrentRun>

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

    connect(&m_backupWatcher, &QFutureWatcher<DM32BackupResult>::finished, this, [this] {
        const auto result = m_backupWatcher.result();
        setBusy(false);
        setOperation(QString());

        if (!result.ok) {
            m_backupReady = false;
            clearChannelState();
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

        const auto decoded = DM32ChannelDecoder::decodeFile(result.backupPath);
        if (decoded.ok) {
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

            setStatus(
                QStringLiteral("RAW BACKUP COMPLETE // %1 CHANNELS DECODED // SHA256 %2")
                    .arg(decoded.channelCount)
                    .arg(result.sha256.left(16).toUpper()));
        } else {
            clearChannelState();
            setStatus(QStringLiteral("RAW BACKUP COMPLETE // CHANNEL DECODE FAILED // %1").arg(decoded.error));
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
    emit radioChanged();

    setReadProgress(0);
    setOperation(QStringLiteral("probe"));
    setBusy(true);
    setStatus(QStringLiteral("PROBING %1 // PSEARCH").arg(portName));

    m_probeWatcher.setFuture(QtConcurrent::run([portName] {
        return DM32Connection::probe(portName);
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
        setStatus(QStringLiteral("RAW BACKUP BLOCKED // PROBE THE SELECTED PORT FIRST"));
        return;
    }

    clearBackupState();
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

void AppController::clearRadio()
{
    if (m_busy) {
        return;
    }

    m_radioDetected = false;
    m_radioModel.clear();
    m_detectedPort.clear();
    clearBackupState();
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
