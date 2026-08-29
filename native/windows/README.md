# YWD-Plug Native Windows

Native Windows CPS for YWD-Plug, built with **C++20 + Qt 6/QML** on the `dev-win` branch.

This is a real native application. It does not embed the browser YWD-Plug UI and it does not require Chromium/Web Serial at runtime.

## Current phase

Milestone 1 is checkpointed at:

```text
checkpoint/dev-win-m1
```

That checkpoint proved the native application shell, Win32 serial transport, DM-32UV / DP570UV identification handshake, terminal-style UI, splash screen, and window-state persistence on real hardware.

`dev-win` is now in **Phase 2: read-only PROGRAM-mode validation**.

Phase 2 currently adds:

1. The proven `PSEARCH` / `PASSSTA` / `SYSINFO` handshake.
2. Firmware V-frame `0x01` read.
3. Memory-layout V-frame `0x0A` read.
4. Validation of the radio-reported config-memory range.
5. PROGRAM-mode entry using the browser-proven sequence.
6. Safe 4096-byte memory reads.
7. A contiguous raw config-memory backup.
8. SHA-256 hashing and a JSON manifest with block metadata.
9. Live read progress in the terminal UI.

**Native radio-memory writes remain unavailable.** There is no native `writeMemory()` implementation in this phase.

See [`PHASE2-READBACK.md`](PHASE2-READBACK.md) for the hardware test procedure and success criteria.

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

For the Phase-2 radio test:

1. Select the correct COM port.
2. Run **EXECUTE PROBE**.
3. Confirm `RADIO DETECTED`.
4. Run **RAW BACKUP**.
5. Leave the radio connected until the read reaches 100%.

Successful raw backups are stored under:

```text
%USERPROFILE%\Documents\YWD-Plug Backups\
```

Each read creates a `.bin` image and a `.json` manifest.

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
├── qml/
├── resources/
└── src/
    ├── app/
    ├── serial/
    │   └── WinSerialPort.*
    └── radios/
        └── dm32uv/
```

## Next validation gate

Do not port native codeplug decoding or radio-memory writes merely because the first raw read completes.

The next gate is:

1. Repeat the native read and prove stable output when the radio configuration is unchanged.
2. Compare the native memory range and block data against the existing browser YWD-Plug read path.
3. Confirm the same block metadata and byte content.
4. Then port channel count and channel decoding into the native application.

Radio-memory writes remain a later milestone after native read/decode/encode round-trip validation.