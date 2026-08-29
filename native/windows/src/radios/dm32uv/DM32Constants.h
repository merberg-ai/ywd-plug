#pragma once

#include <QtGlobal>

namespace DM32Constants {
inline constexpr qint32 BaudRate = 115200;
inline constexpr int InitDelayMs = 400;
inline constexpr int ClearBufferDelayMs = 200;
inline constexpr int PsearchReadDelayMs = 150;
inline constexpr int RequestTimeoutMs = 5000;
inline constexpr int ReadMemoryTimeoutMs = 15000;
inline constexpr int BlockReadDelayMs = 150;
inline constexpr qsizetype BlockSize = 4096;
}