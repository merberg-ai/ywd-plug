# YWD-Plug Native Windows

Native Windows CPS for YWD-Plug, built with **C++20 + Qt 6/QML** on the `dev-win` branch.

This is intentionally a real native application. It does not embed the browser YWD-Plug UI and it does not require Chromium/Web Serial at runtime.

## Current milestone

Milestone 1 is deliberately read-only and boring in the best possible way:

1. Start a native Qt 6 application.
2. Enumerate Windows COM ports with the native Win32 API.
3. Open the selected port at 115200 8N1 using the native Win32 COM API.
4. Perform the proven DM-32UV identification sequence:
   - `PSEARCH`
   - `PASSSTA`
   - `SYSINFO`
5. Confirm a `DP570`, `DM32`, or `DM-32` model response.
6. Close the port.

**This milestone does not enter PROGRAM mode and does not write radio memory.**

The handshake timing and validation are ported from the existing working browser DM-32UV driver so the native implementation starts from known behavior rather than a fresh reverse-engineering attempt.

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

When the first radio probe works, install the current-user build with:

```bat
INSTALL.cmd
```

`INSTALL.cmd` copies the staged application to:

```text
%LOCALAPPDATA%\Programs\YWD-Plug
```

and creates Start Menu and Desktop shortcuts. Administrator access is not required.

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
├── qml/
├── resources/
└── src/
    ├── app/
    ├── serial/
    │   └── WinSerialPort.*
    └── radios/
        └── dm32uv/
```

## Next milestone

After the hardware probe is confirmed on a DM-32UV, port the programming-mode entry and safe memory block reads. Only after raw reads and binary comparisons are proven should native radio writes be enabled.

Longer term, the native application should preserve `.ywdplug` compatibility with the browser version so codeplugs can move between both front ends.
