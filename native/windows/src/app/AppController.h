#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QVariantList>

#include "radios/dm32uv/DM32Connection.h"

struct DM32ChannelDecodeResult;
struct DM32CodeplugDecodeResult;

class AppController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString radioModel READ radioModel NOTIFY radioChanged)
    Q_PROPERTY(QString detectedPort READ detectedPort NOTIFY radioChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool radioDetected READ radioDetected NOTIFY radioChanged)
    Q_PROPERTY(int readProgress READ readProgress NOTIFY readProgressChanged)
    Q_PROPERTY(QString operation READ operation NOTIFY operationChanged)
    Q_PROPERTY(bool backupReady READ backupReady NOTIFY backupChanged)
    Q_PROPERTY(QString backupPath READ backupPath NOTIFY backupChanged)
    Q_PROPERTY(QString backupManifestPath READ backupManifestPath NOTIFY backupChanged)
    Q_PROPERTY(QString backupSha256 READ backupSha256 NOTIFY backupChanged)
    Q_PROPERTY(QVariantList channels READ channels NOTIFY channelsChanged)
    Q_PROPERTY(int channelCount READ channelCount NOTIFY channelsChanged)
    Q_PROPERTY(bool channelsReady READ channelsReady NOTIFY channelsChanged)
    Q_PROPERTY(QVariantList zones READ zones NOTIFY codeplugChanged)
    Q_PROPERTY(int zoneCount READ zoneCount NOTIFY codeplugChanged)
    Q_PROPERTY(QVariantList scanLists READ scanLists NOTIFY codeplugChanged)
    Q_PROPERTY(int scanListCount READ scanListCount NOTIFY codeplugChanged)
    Q_PROPERTY(QVariantList rxGroups READ rxGroups NOTIFY codeplugChanged)
    Q_PROPERTY(int rxGroupCount READ rxGroupCount NOTIFY codeplugChanged)
    Q_PROPERTY(bool codeplugReady READ codeplugReady NOTIFY codeplugChanged)
    Q_PROPERTY(bool liveReadReady READ liveReadReady NOTIFY readStatsChanged)
    Q_PROPERTY(qint64 lastReadBytes READ lastReadBytes NOTIFY readStatsChanged)
    Q_PROPERTY(qint64 lastReadMs READ lastReadMs NOTIFY readStatsChanged)
    Q_PROPERTY(int lastReadDataBlocks READ lastReadDataBlocks NOTIFY readStatsChanged)
    Q_PROPERTY(int lastReadDiscoveredBlocks READ lastReadDiscoveredBlocks NOTIFY readStatsChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    [[nodiscard]] QVariantList ports() const { return m_ports; }
    [[nodiscard]] QString status() const { return m_status; }
    [[nodiscard]] QString radioModel() const { return m_radioModel; }
    [[nodiscard]] QString detectedPort() const { return m_detectedPort; }
    [[nodiscard]] bool busy() const { return m_busy; }
    [[nodiscard]] bool radioDetected() const { return m_radioDetected; }
    [[nodiscard]] int readProgress() const { return m_readProgress; }
    [[nodiscard]] QString operation() const { return m_operation; }
    [[nodiscard]] bool backupReady() const { return m_backupReady; }
    [[nodiscard]] QString backupPath() const { return m_backupPath; }
    [[nodiscard]] QString backupManifestPath() const { return m_backupManifestPath; }
    [[nodiscard]] QString backupSha256() const { return m_backupSha256; }
    [[nodiscard]] QVariantList channels() const { return m_channels; }
    [[nodiscard]] int channelCount() const { return m_channelCount; }
    [[nodiscard]] bool channelsReady() const { return m_channelsReady; }
    [[nodiscard]] QVariantList zones() const { return m_zones; }
    [[nodiscard]] int zoneCount() const { return m_zones.size(); }
    [[nodiscard]] QVariantList scanLists() const { return m_scanLists; }
    [[nodiscard]] int scanListCount() const { return m_scanLists.size(); }
    [[nodiscard]] QVariantList rxGroups() const { return m_rxGroups; }
    [[nodiscard]] int rxGroupCount() const { return m_rxGroups.size(); }
    [[nodiscard]] bool codeplugReady() const { return m_codeplugReady; }
    [[nodiscard]] bool liveReadReady() const { return m_liveReadReady; }
    [[nodiscard]] qint64 lastReadBytes() const { return m_lastReadBytes; }
    [[nodiscard]] qint64 lastReadMs() const { return m_lastReadMs; }
    [[nodiscard]] int lastReadDataBlocks() const { return m_lastReadDataBlocks; }
    [[nodiscard]] int lastReadDiscoveredBlocks() const { return m_lastReadDiscoveredBlocks; }

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void probePort(const QString &portName);
    Q_INVOKABLE void readRadio(const QString &portName);
    Q_INVOKABLE void readRawBackup(const QString &portName);
    Q_INVOKABLE void loadLatestBackup();
    Q_INVOKABLE void clearRadio();

signals:
    void portsChanged();
    void statusChanged();
    void radioChanged();
    void busyChanged();
    void readProgressChanged();
    void operationChanged();
    void backupChanged();
    void channelsChanged();
    void codeplugChanged();
    void readStatsChanged();

private:
    void setStatus(const QString &status);
    void setBusy(bool busy);
    void setReadProgress(int progress);
    void setOperation(const QString &operation);
    void clearBackupState();
    void clearChannelState();
    void clearCodeplugState();
    void clearReadStats();
    bool applyDecodedChannels(const DM32ChannelDecodeResult &decoded, QString &error);
    bool applyDecodedCodeplug(const DM32CodeplugDecodeResult &decoded, QString &error);
    bool loadCodeplugFromBackup(const QString &path, QString &error);

    QVariantList m_ports;
    QVariantList m_channels;
    QVariantList m_zones;
    QVariantList m_scanLists;
    QVariantList m_rxGroups;
    QString m_status {QStringLiteral("READY // SELECT A COM PORT")};
    QString m_radioModel;
    QString m_detectedPort;
    QString m_operation;
    QString m_backupPath;
    QString m_backupManifestPath;
    QString m_backupSha256;
    bool m_busy {false};
    bool m_radioDetected {false};
    bool m_backupReady {false};
    bool m_channelsReady {false};
    bool m_codeplugReady {false};
    bool m_liveReadReady {false};
    int m_readProgress {0};
    int m_channelCount {0};
    qint64 m_lastReadBytes {0};
    qint64 m_lastReadMs {0};
    int m_lastReadDataBlocks {0};
    int m_lastReadDiscoveredBlocks {0};
    QFutureWatcher<DM32ProbeResult> m_probeWatcher;
    QFutureWatcher<DM32SelectiveReadResult> m_selectiveReadWatcher;
    QFutureWatcher<DM32BackupResult> m_backupWatcher;
};
