#include "AppController.h"

#include <QSerialPortInfo>
#include <QtConcurrent/QtConcurrentRun>

namespace {
QString hexId(quint16 value, bool present)
{
    if (!present) {
        return QStringLiteral("----");
    }
    return QStringLiteral("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper();
}
}

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

    for (const auto &info : QSerialPortInfo::availablePorts()) {
        QVariantMap item;
        item.insert(QStringLiteral("name"), info.portName());
        item.insert(QStringLiteral("description"), info.description());
        item.insert(QStringLiteral("manufacturer"), info.manufacturer());
        item.insert(QStringLiteral("serialNumber"), info.serialNumber());
        item.insert(QStringLiteral("vendorId"), hexId(info.vendorIdentifier(), info.hasVendorIdentifier()));
        item.insert(QStringLiteral("productId"), hexId(info.productIdentifier(), info.hasProductIdentifier()));

        QString label = info.portName();
        if (!info.description().isEmpty()) {
            label += QStringLiteral(" — ") + info.description();
        }
        if (info.hasVendorIdentifier() || info.hasProductIdentifier()) {
            label += QStringLiteral("  [%1:%2]")
                         .arg(item.value(QStringLiteral("vendorId")).toString(),
                              item.value(QStringLiteral("productId")).toString());
        }
        item.insert(QStringLiteral("label"), label);
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
