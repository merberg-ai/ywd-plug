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

public:
    explicit AppController(QObject *parent = nullptr);

    [[nodiscard]] QVariantList ports() const { return m_ports; }
    [[nodiscard]] QString status() const { return m_status; }
    [[nodiscard]] QString radioModel() const { return m_radioModel; }
    [[nodiscard]] QString detectedPort() const { return m_detectedPort; }
    [[nodiscard]] bool busy() const { return m_busy; }
    [[nodiscard]] bool radioDetected() const { return m_radioDetected; }

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void probePort(const QString &portName);
    Q_INVOKABLE void clearRadio();

signals:
    void portsChanged();
    void statusChanged();
    void radioChanged();
    void busyChanged();

private:
    void setStatus(const QString &status);
    void setBusy(bool busy);

    QVariantList m_ports;
    QString m_status {QStringLiteral("READY // SELECT A COM PORT")};
    QString m_radioModel;
    QString m_detectedPort;
    bool m_busy {false};
    bool m_radioDetected {false};
    QFutureWatcher<DM32ProbeResult> m_probeWatcher;
};
