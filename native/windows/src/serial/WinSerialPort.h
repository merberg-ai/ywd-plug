#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QtGlobal>

class WinSerialPort final
{
public:
    WinSerialPort() = default;
    ~WinSerialPort();

    WinSerialPort(const WinSerialPort &) = delete;
    WinSerialPort &operator=(const WinSerialPort &) = delete;

    static QStringList availablePorts();

    bool open(const QString &portName, qint32 baudRate);
    void close();
    bool clear();
    bool writeAll(const QByteArray &data, int timeoutMs, QString &error);
    QByteArray readExact(qsizetype count, int timeoutMs);

    [[nodiscard]] QString errorString() const { return m_error; }

private:
    void setLastError(const QString &context);

    void *m_handle {nullptr};
    QString m_error;
};
