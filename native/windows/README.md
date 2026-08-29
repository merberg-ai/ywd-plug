# YWD-Plug Native Windows

Native Windows CPS for YWD-Plug, built with **C++20 + Qt 6/QML** on the `dev-win` branch.

This is a real native application. It does not embed the browser YWD-Plug UI and it does not require Chromium/Web Serial at runtime.

## Current phase

Milestone 1 is checkpointed at:

```text
checkpoint/dev-win-m1
```

The hardware-validated native channel decoder / offline backup state is checkpointed at:

```text
checkpoint/dev-win-m2b
```

`dev-win` is now in **Phase 3: selective live READ RADIO**.

Phase 3 includes:

1. Native `PSEARCH` / `PASSSTA` / `SYSINFO` handshake.
2. Firmware V-frame `0x01` and memory-layout V-frame `0x0A` reads.
3. PROGRAM-mode entry using the browser-proven read-only sequence.
4. Metadata discovery by reading only byte `0xFFF` from each 4 KiB config block.
5. Channel-count read from metadata block `0x12`.
6. Exact selection of only the channel blocks required by the current channel count.
7. TX-contact block reads (`0x42`, and `0x43` when required).
8. In-memory native C++ channel decoding directly into the Channels workspace.
9. A measured **READ RADIO** path reporting transferred bytes, discovered blocks, data blocks, and elapsed time.
10. The existing exhaustive **RAW BACKUP** path, SHA-256 manifest, and **LOAD BACKUP** offline workflow.

**Native radio-memory writes remain unavailable and editing remains locked.**

See:

- [`PHASE2-READBACK.md`](PHASE2-READBACK.md) — exhaustive raw backup validation
- [`PHASE3-SELECTIVE-READ.md`](PHASE3-SELECTIVE-READ.md) — normal fast READ RADIO validation

## Stack

- C++20
- Qt 6.8+ Core / GUI / Quick / Quick Controls / Concurrent
- Qt Quick / QML
- Native Win32 COM transport (`CreateFile`, `SetCommState`, `ReadFile`, `WriteFile`)
- CMake
- Ninja
- MSVC 2022 x64

Qt SerialPort is intentionally **not required**. The Windows build owns its COM transport directly so a normal Qt MSVC desktop installation is enough.

## Easy path

Open a normal Windows Terminal / Command Prompt in this directory and run:

```bat
SETUP.cmd
BUILD.cmd
RUN.cmd
```

For the normal Phase-3 radio test:

1. Select the correct COM port.
2. Click **READ RADIO** directly; Probe is optional.
3. Watch metadata discovery and selective block-read progress.
4. On success, the Channels workspace should populate from the live radio.
5. Compare the final byte count / elapsed time with the exhaustive RAW BACKUP path.

For the known 88-channel test codeplug, the expected payload is about **12,490 bytes** rather than the 819,200-byte raw image.

## Raw backups

**RAW BACKUP** remains the exhaustive safety/diagnostic operation. Successful raw backups are stored under:

```text
%USERPROFILE%\Documents\YWD-Plug Backups\
```

Each raw backup creates a `.bin` image and a `.json` manifest.

**LOAD BACKUP** decodes the newest saved image without touching the radio.

## Install

When the current build is ready to install for the current user:

```bat
INSTALL.cmd
```

The application is copied to:

```text
%LOCALAPPDATA%\Programs\YWD-Plug
```

and Start Menu and Desktop shortcuts are created. Administrator access is not required.

## Toolchain setup

`SETUP.cmd` checks for:

- Git
- CMake
- Ninja
- Visual Studio 2022 / Build Tools with the MSVC x64 C++ toolchain
- Qt 6.8+ MSVC x64

The launcher looks first at `QTDIR`, then `Qt6_DIR`, then common `C:\Qt\<version>\msvc*_64` installs.

To let the setup script install common command-line tools through winget when missing:

```bat
SETUP.cmd -InstallMissing
```

Visual Studio C++ and Qt are intentionally not installed silently by the script. Qt should include an **MSVC 2022 64-bit** kit. The optional Qt SerialPort package is not needed.

## Build output

- `build\` — CMake/Ninja build tree
- `dist\` — portable app plus deployed Qt runtime/QML modules

Both are gitignored.

## Native source layout

```text
native/windows/
├── CMakeLists.txt
├── YWD-PLUG.ps1
├── SETUP.cmd
├── BUILD.cmd
├── INSTALL.cmd
├── RUN.cmd
├── CHECKPOINT-M1.md
├── PHASE2-READBACK.md
├── PHASE3-SELECTIVE-READ.md
├── qml/
├── resources/
└── src/
    ├── app/
    ├── serial/
    │   └── WinSerialPort.*
    └── radios/
        └── dm32uv/
            ├── DM32Connection.*
            ├── DM32SelectiveRead.cpp
            ├── DM32MemoryBlock.h
            └── DM32ChannelDecoder.*
```

## Next validation gate

Phase 3 must be hardware-proven before expanding the CPS read model.

After selective READ RADIO is validated:

1. Reuse the metadata map + selected-block reader for Zones.
2. Add Scan Lists.
3. Add RX Groups.
4. Add Contacts / Talk Groups and DMR Radio IDs.
5. Add Settings / Display.
6. Checkpoint the complete read-only native CPS.
7. Only then begin binary round-trip encode verification and the later radio-write milestone.

Radio-memory writes remain a later milestone after native read/decode/encode round-trip validation.
