#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QVariantList>

#include "radios/dm32uv/DM32Connection.h"

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

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void probePort(const QString &portName);
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

private:
    void setStatus(const QString &status);
    void setBusy(bool busy);
    void setReadProgress(int progress);
    void setOperation(const QString &operation);
    void clearBackupState();
    void clearChannelState();
    bool loadChannelsFromBackup(const QString &path, QString &error);

    QVariantList m_ports;
    QVariantList m_channels;
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
    int m_readProgress {0};
    int m_channelCount {0};
    QFutureWatcher<DM32ProbeResult> m_probeWatcher;
    QFutureWatcher<DM32BackupResult> m_backupWatcher;
};
