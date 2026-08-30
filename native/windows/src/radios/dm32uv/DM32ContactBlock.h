#pragma once

#include <QByteArray>
#include <QtGlobal>

struct DM32ContactBlock
{
    quint32 address {0};
    int blockNumber {0};
    QByteArray data;
};
