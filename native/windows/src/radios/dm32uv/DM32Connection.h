#pragma once

#include <QByteArray>
#include <QString>

struct DM32ProbeResult
{
    bool ok {false};
    QString model;
    QString portName;
    QString error;
    QByteArray psearchResponse;
};

class DM32Connection final
{
public:
    static DM32ProbeResult probe(const QString &portName);
};
