#include "AppController.h"

#include "serial/WinSerialPort.h"

#include <QtConcurrent/QtConcurrentRun>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    connect(&m_probeWatcher, &QFutureWatcher<DM32ProbeResult>::finished, this, [this] {
        const auto result = m_probeWatcher.result();
        setBusy(false);

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
    emit radioChanged();

    setBusy(true);
    setStatus(QStringLiteral("PROBING %1 // PSEARCH").arg(portName));

    m_probeWatcher.setFuture(QtConcurrent::run([portName] {
        return DM32Connection::probe(portName);
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
