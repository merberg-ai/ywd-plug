#pragma once

#include "DM32MemoryBlock.h"

#include <QByteArray>
#include <QString>
#include <QVector>

#include <functional>

struct DM32ProbeResult
{
    bool ok {false};
    QString model;
    QString portName;
    QString error;
    QByteArray psearchResponse;
};

struct DM32BackupResult
{
    bool ok {false};
    QString model;
    QString portName;
    QString firmware;
    QString error;
    QString backupPath;
    QString manifestPath;
    QString sha256;
    quint32 configStart {0};
    quint32 configEnd {0};
    int blockCount {0};
    qint64 bytesRead {0};
};

struct DM32SelectiveReadResult
{
    bool ok {false};
    QString model;
    QString portName;
    QString firmware;
    QString error;
    quint32 configStart {0};
    quint32 configEnd {0};
    int discoveredBlockCount {0};
    int channelCount {0};
    int dataBlockCount {0};
    qint64 bytesTransferred {0};
    qint64 elapsedMs {0};
    QVector<DM32MemoryBlock> blocks;
};

class DM32Connection final
{
public:
    using ProgressCallback = std::function<void(int progress, const QString &message)>;

    static DM32ProbeResult probe(const QString &portName);
    static DM32SelectiveReadResult readChannelsSelective(
        const QString &portName,
        const ProgressCallback &onProgress = {});
    static DM32BackupResult readRawBackup(
        const QString &portName,
        const QString &outputDirectory,
        const ProgressCallback &onProgress = {});
};
