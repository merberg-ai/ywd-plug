#include "WinSerialPort.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <QElapsedTimer>

#include <array>

namespace {
HANDLE nativeHandle(void *handle)
{
    return static_cast<HANDLE>(handle);
}

QString windowsErrorText(DWORD code)
{
    std::array<wchar_t, 512> buffer {};
    const DWORD length = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer.data(),
        static_cast<DWORD>(buffer.size()),
        nullptr);

    QString text = length > 0
        ? QString::fromWCharArray(buffer.data(), static_cast<qsizetype>(length)).trimmed()
        : QStringLiteral("Windows error %1").arg(code);

    return text;
}

bool setTimeouts(HANDLE handle, DWORD readConstantMs, DWORD writeConstantMs)
{
    COMMTIMEOUTS timeouts {};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = readConstantMs;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = writeConstantMs;
    return SetCommTimeouts(handle, &timeouts) != FALSE;
}
}

WinSerialPort::~WinSerialPort()
{
    close();
}

QStringList WinSerialPort::availablePorts()
{
    QStringList ports;
    std::array<wchar_t, 4096> target {};

    for (int number = 1; number <= 256; ++number) {
        const QString name = QStringLiteral("COM%1").arg(number);
        SetLastError(ERROR_SUCCESS);
        const DWORD result = QueryDosDeviceW(
            reinterpret_cast<LPCWSTR>(name.utf16()),
            target.data(),
            static_cast<DWORD>(target.size()));

        if (result != 0) {
            ports.push_back(name);
        }
    }

    return ports;
}

bool WinSerialPort::open(const QString &portName, qint32 baudRate)
{
    close();
    m_error.clear();

    const QString devicePath = QStringLiteral("\\\\.\\%1").arg(portName.trimmed());
    HANDLE handle = CreateFileW(
        reinterpret_cast<LPCWSTR>(devicePath.utf16()),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        m_handle = nullptr;
        setLastError(QStringLiteral("Could not open %1").arg(portName));
        return false;
    }

    m_handle = handle;

    if (SetupComm(handle, 8192, 8192) == FALSE) {
        setLastError(QStringLiteral("Could not configure serial buffers"));
        close();
        return false;
    }

    DCB dcb {};
    dcb.DCBlength = sizeof(DCB);
    if (GetCommState(handle, &dcb) == FALSE) {
        setLastError(QStringLiteral("Could not read serial configuration"));
        close();
        return false;
    }

    dcb.BaudRate = static_cast<DWORD>(baudRate);
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fAbortOnError = FALSE;

    if (SetCommState(handle, &dcb) == FALSE) {
        setLastError(QStringLiteral("Could not apply 115200 8N1 serial configuration"));
        close();
        return false;
    }

    if (!setTimeouts(handle, 100, 5000)) {
        setLastError(QStringLiteral("Could not configure serial timeouts"));
        close();
        return false;
    }

    if (!clear()) {
        close();
        return false;
    }

    return true;
}

void WinSerialPort::close()
{
    if (!m_handle) {
        return;
    }

    HANDLE handle = nativeHandle(m_handle);
    PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
    CloseHandle(handle);
    m_handle = nullptr;
}

bool WinSerialPort::clear()
{
    if (!m_handle) {
        m_error = QStringLiteral("Serial port is not open");
        return false;
    }

    if (PurgeComm(nativeHandle(m_handle), PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR) == FALSE) {
        setLastError(QStringLiteral("Could not clear serial buffers"));
        return false;
    }

    return true;
}

bool WinSerialPort::writeAll(const QByteArray &data, int timeoutMs, QString &error)
{
    error.clear();
    if (!m_handle) {
        error = QStringLiteral("Serial port is not open");
        return false;
    }

    HANDLE handle = nativeHandle(m_handle);
    if (!setTimeouts(handle, 100, static_cast<DWORD>(qMax(1, timeoutMs)))) {
        setLastError(QStringLiteral("Could not configure write timeout"));
        error = m_error;
        return false;
    }

    QElapsedTimer timer;
    timer.start();
    qsizetype offset = 0;

    while (offset < data.size()) {
        if (timer.elapsed() >= timeoutMs) {
            error = QStringLiteral("Serial write timed out after %1 ms").arg(timeoutMs);
            return false;
        }

        DWORD written = 0;
        const DWORD request = static_cast<DWORD>(qMin<qsizetype>(data.size() - offset, 64 * 1024));
        if (WriteFile(handle, data.constData() + offset, request, &written, nullptr) == FALSE) {
            setLastError(QStringLiteral("Serial write failed"));
            error = m_error;
            return false;
        }

        if (written == 0) {
            continue;
        }

        offset += static_cast<qsizetype>(written);
    }

    if (FlushFileBuffers(handle) == FALSE) {
        setLastError(QStringLiteral("Could not flush serial write"));
        error = m_error;
        return false;
    }

    return true;
}

QByteArray WinSerialPort::readExact(qsizetype count, int timeoutMs)
{
    QByteArray data;
    data.reserve(count);

    if (!m_handle) {
        m_error = QStringLiteral("Serial port is not open");
        return data;
    }

    HANDLE handle = nativeHandle(m_handle);
    QElapsedTimer timer;
    timer.start();

    std::array<char, 4096> buffer {};

    while (data.size() < count && timer.elapsed() < timeoutMs) {
        const int remainingMs = qMax(1, timeoutMs - static_cast<int>(timer.elapsed()));
        const DWORD sliceMs = static_cast<DWORD>(qMin(remainingMs, 250));
        if (!setTimeouts(handle, sliceMs, static_cast<DWORD>(timeoutMs))) {
            setLastError(QStringLiteral("Could not configure read timeout"));
            break;
        }

        const DWORD request = static_cast<DWORD>(qMin<qsizetype>(count - data.size(), static_cast<qsizetype>(buffer.size())));
        DWORD received = 0;
        if (ReadFile(handle, buffer.data(), request, &received, nullptr) == FALSE) {
            setLastError(QStringLiteral("Serial read failed"));
            break;
        }

        if (received > 0) {
            data.append(buffer.data(), static_cast<qsizetype>(received));
        }
    }

    return data;
}

void WinSerialPort::setLastError(const QString &context)
{
    const DWORD code = GetLastError();
    m_error = QStringLiteral("%1: %2 (0x%3)")
                  .arg(context,
                       windowsErrorText(code),
                       QString::number(code, 16).toUpper().rightJustified(8, QLatin1Char('0')));
}
