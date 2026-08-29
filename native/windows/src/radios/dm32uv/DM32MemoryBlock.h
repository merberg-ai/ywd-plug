#pragma once

#include <QByteArray>
#include <QtGlobal>

struct DM32MemoryBlock
{
    quint32 address {0};
    quint8 metadata {0};
    QByteArray data;
};
