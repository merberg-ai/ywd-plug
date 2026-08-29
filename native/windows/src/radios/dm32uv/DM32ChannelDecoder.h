#pragma once

#include "DM32MemoryBlock.h"

#include <QString>
#include <QVector>

struct DM32ChannelInfo
{
    int number {0};
    QString name;
    double rxFrequency {0.0};
    double txFrequency {0.0};
    bool txDisabled {false};
    QString mode;
    QString power;
    QString bandwidth;
    bool scanAdd {false};
    int scanListId {0};
    int colorCode {-1};
    int timeSlot {0};
    int rxGroupListId {-1};
    int txContactIndex {-1};
    bool txContactDigital {false};
};

struct DM32ChannelDecodeResult
{
    bool ok {false};
    QString error;
    int channelCount {0};
    QVector<DM32ChannelInfo> channels;
};

class DM32ChannelDecoder final
{
public:
    static DM32ChannelDecodeResult decodeFile(const QString &path);
    static DM32ChannelDecodeResult decodeBlocks(const QVector<DM32MemoryBlock> &blocks);
};
