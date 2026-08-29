#include "DM32Connection.h"

#include "DM32Constants.h"

#include <QElapsedTimer>
#include <QSerialPort>
#include <QThread>

namespace {
bool writeAll(QSerialPort &port, const QByteArray &data, QString &error)
{
    const auto queued = port.write(data);
    if (queued != data.size()) {
        error = QStringLiteral("Could not queue serial write: %1").arg(port.errorString());
        return false;
    }

    if (!port.waitForBytesWritten(DM32Constants::RequestTimeoutMs)) {
        error = QStringLiteral("Serial write timed out: %1").arg(port.errorString());
        return false;
    }
    return true;
}

QByteArray readExact(QSerialPort &port, qsizetype count, int timeoutMs)
{
    QByteArray data;
    QElapsedTimer timer;
    timer.start();

    while (data.size() < count && timer.elapsed() < timeoutMs) {
        if (port.bytesAvailable() == 0) {
            const int remaining = qMax(1, timeoutMs - static_cast<int>(timer.elapsed()));
            port.waitForReadyRead(qMin(remaining, 250));
        }

        const auto chunk = port.read(count - data.size());
        if (!chunk.isEmpty()) {
            data.append(chunk);
        }
    }

    return data;
}

QString toHex(const QByteArray &data)
{
    return QString::fromLatin1(data.toHex(' ')).toUpper();
}

bool hasSupportedModel(const QString &model)
{
    return model.contains(QStringLiteral("DP570"), Qt::CaseInsensitive)
        || model.contains(QStringLiteral("DM32"), Qt::CaseInsensitive)
        || model.contains(QStringLiteral("DM-32"), Qt::CaseInsensitive);
}
}

DM32ProbeResult DM32Connection::probe(const QString &portName)
{
    DM32ProbeResult result;
    result.portName = portName;

    QSerialPort port;
    port.setPortName(portName);
    port.setBaudRate(DM32Constants::BaudRate);
    port.setDataBits(QSerialPort::Data8);
    port.setParity(QSerialPort::NoParity);
    port.setStopBits(QSerialPort::OneStop);
    port.setFlowControl(QSerialPort::NoFlowControl);

    if (!port.open(QIODevice::ReadWrite)) {
        result.error = QStringLiteral("Could not open %1: %2").arg(portName, port.errorString());
        return result;
    }

    QThread::msleep(DM32Constants::InitDelayMs);
    port.clear(QSerialPort::AllDirections);
    QThread::msleep(DM32Constants::ClearBufferDelayMs);

    QString ioError;
    if (!writeAll(port, QByteArrayLiteral("PSEARCH"), ioError)) {
        result.error = ioError;
        return result;
    }

    QThread::msleep(DM32Constants::PsearchReadDelayMs);
    const auto psearch = readExact(port, 8, DM32Constants::RequestTimeoutMs);
    result.psearchResponse = psearch;

    if (psearch.size() != 8) {
        result.error = QStringLiteral("PSEARCH timeout: expected 8 bytes, received %1 [%2]")
                           .arg(psearch.size())
                           .arg(toHex(psearch));
        return result;
    }

    if (static_cast<quint8>(psearch.at(0)) != 0x06) {
        result.error = QStringLiteral("PSEARCH rejected: expected ACK 06, received [%1]").arg(toHex(psearch));
        return result;
    }

    QString model = QString::fromLatin1(psearch.mid(1));
    model.remove(QChar::Null);
    model = model.trimmed();

    if (!hasSupportedModel(model)) {
        result.error = QStringLiteral("Unsupported radio response: %1 [%2]").arg(model, toHex(psearch));
        return result;
    }

    QThread::msleep(50);
    if (!writeAll(port, QByteArrayLiteral("PASSSTA"), ioError)) {
        result.error = ioError;
        return result;
    }
    QThread::msleep(50);

    const auto passsta = readExact(port, 3, DM32Constants::RequestTimeoutMs);
    if (passsta.size() != 3 || static_cast<quint8>(passsta.at(0)) != 0x50) {
        result.error = QStringLiteral("PASSSTA failed: [%1]").arg(toHex(passsta));
        return result;
    }

    QThread::msleep(50);
    if (!writeAll(port, QByteArrayLiteral("SYSINFO"), ioError)) {
        result.error = ioError;
        return result;
    }
    QThread::msleep(50);

    const auto sysinfo = readExact(port, 1, DM32Constants::RequestTimeoutMs);
    if (sysinfo.size() != 1 || static_cast<quint8>(sysinfo.at(0)) != 0x06) {
        result.error = QStringLiteral("SYSINFO failed: [%1]").arg(toHex(sysinfo));
        return result;
    }

    result.ok = true;
    result.model = model;
    return result;
}
