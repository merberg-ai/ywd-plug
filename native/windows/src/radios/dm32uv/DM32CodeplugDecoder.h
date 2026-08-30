#pragma once

#include "DM32MemoryBlock.h"

#include <QString>
#include <QVector>

struct DM32ZoneInfo
{
    int number {0};
    QString name;
    QVector<int> channels;
};

struct DM32ScanListInfo
{
    int number {0};
    QString name;
    QVector<int> channels;
    double hangTimeSeconds {0.0};
    int priority1Type {0};
    int priority1Channel {-1};
    int priority2Type {0};
    int priority2Channel {-1};
    int designatedTxMode {0};
    int designatedTxChannel {-1};
};

struct DM32RxGroupInfo
{
    int number {0};
    QString name;
    QVector<quint32> memberIds;
};

struct DM32CodeplugDecodeResult
{
    bool ok {false};
    QString error;
    QVector<DM32ZoneInfo> zones;
    QVector<DM32ScanListInfo> scanLists;
    QVector<DM32RxGroupInfo> rxGroups;
};

class DM32CodeplugDecoder final
{
public:
    static DM32CodeplugDecodeResult decodeBlocks(const QVector<DM32MemoryBlock> &blocks);
    static DM32CodeplugDecodeResult decodeFile(const QString &path);
};
