#pragma once

#include "DM32ContactBlock.h"

#include <QString>
#include <QVector>
#include <QtGlobal>

struct DM32ContactInfo
{
    int index {0};
    QString name;
    quint32 dmrId {0};
    QString callSign;
    QString city;
    QString province;
    QString country;
    QString remark;
};

struct DM32ContactDecodeResult
{
    bool ok {false};
    QString error;
    int databaseCount {0};
    QVector<DM32ContactInfo> contacts;
};

class DM32ContactDecoder final
{
public:
    static DM32ContactDecodeResult decodeReferenced(
        const QVector<DM32ContactBlock> &blocks,
        quint32 contactBase,
        int databaseCount,
        const QVector<int> &referencedContactIds);
};
