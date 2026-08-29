#include "DM32Connection.h"

#include "DM32Constants.h"
#include "serial/WinSerialPort.h"

#include <QThread>

namespace {
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
}

DM32ProbeResult DM32Connection::probe(const QString &portName)
{
    DM32ProbeResult result;
    result.portName = portName;

    WinSerialPort port;
    if (!port.open(portName, DM32Constants::BaudRate)) {
        result.error = port.errorString();
        return result;
    }

    QThread::msleep(DM32Constants::InitDelayMs);
    if (!port.clear()) {
        result.error = port.errorString();
        return result;
    }
    QThread::msleep(DM32Constants::ClearBufferDelayMs);

    QString ioError;
    if (!port.writeAll(QByteArrayLiteral("PSEARCH"), DM32Constants::RequestTimeoutMs, ioError)) {
        result.error = ioError;
        return result;
    }

    QThread::msleep(DM32Constants::PsearchReadDelayMs);
    const auto psearch = port.readExact(8, DM32Constants::RequestTimeoutMs);
    result.psearchResponse = psearch;

    if (psearch.size() != 8) {
        result.error = shortReadError(QStringLiteral("PSEARCH"), 8, psearch, port);
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
    if (!port.writeAll(QByteArrayLiteral("PASSSTA"), DM32Constants::RequestTimeoutMs, ioError)) {
        result.error = ioError;
        return result;
    }
    QThread::msleep(50);

    const auto passsta = port.readExact(3, DM32Constants::RequestTimeoutMs);
    if (passsta.size() != 3) {
        result.error = shortReadError(QStringLiteral("PASSSTA"), 3, passsta, port);
        return result;
    }
    if (static_cast<quint8>(passsta.at(0)) != 0x50) {
        result.error = QStringLiteral("PASSSTA failed: [%1]").arg(toHex(passsta));
        return result;
    }

    QThread::msleep(50);
    if (!port.writeAll(QByteArrayLiteral("SYSINFO"), DM32Constants::RequestTimeoutMs, ioError)) {
        result.error = ioError;
        return result;
    }
    QThread::msleep(50);

    const auto sysinfo = port.readExact(1, DM32Constants::RequestTimeoutMs);
    if (sysinfo.size() != 1) {
        result.error = shortReadError(QStringLiteral("SYSINFO"), 1, sysinfo, port);
        return result;
    }
    if (static_cast<quint8>(sysinfo.at(0)) != 0x06) {
        result.error = QStringLiteral("SYSINFO failed: [%1]").arg(toHex(sysinfo));
        return result;
    }

    result.ok = true;
    result.model = model;
    return result;
}
