# YWD-Plug Native Windows — Milestone 1 Checkpoint

Checkpoint date: 2026-08-29
Branch: `dev-win`

## Status

Milestone 1 is complete and hardware-validated as the native Windows foundation for YWD-Plug.

The application is a true Windows desktop application built with C++20, Qt 6/QML, MSVC, CMake, and Ninja. It does not embed Chromium or the browser YWD-Plug UI.

## Verified on real hardware

- Native application builds successfully with Qt 6.8.0 / MSVC 2022 x64.
- Qt SerialPort is not required.
- Native Win32 COM enumeration works.
- Native Win32 serial transport works at 115200 8N1.
- Baofeng DM-32UV / DP570UV identification works over the programming cable.
- Proven identification sequence succeeds:
  - `PSEARCH`
  - `PASSSTA`
  - `SYSINFO`
- Tested radio response identified successfully as `DP570UV`.
- Probe remains read-only and closes without entering PROGRAM mode.
- Native dark Windows title bar/chrome works.
- Terminal-workstation UI is functioning with graphite/black, silver, amber accent, green success, and red failure states.
- Startup splash/POST screen is implemented.
- Window size and position persist.
- Maximized state persists.
- Minimized state is intentionally transient and is not restored at startup.

## Safety boundary at this checkpoint

Radio writes are still hard-disabled.

Milestone 1 does **not**:

- enter PROGRAM mode for normal codeplug access;
- read arbitrary codeplug memory blocks;
- modify radio memory;
- write codeplug data;
- flash firmware.

This is intentional. The next phase starts by extending the proven read path before any write capability is introduced.

## Native architecture

```text
native/windows/
├── CMakeLists.txt
├── YWD-PLUG.ps1
├── SETUP.cmd
├── BUILD.cmd
├── INSTALL.cmd
├── RUN.cmd
├── qml/
│   ├── Main.qml
│   ├── SplashWindow.qml
│   └── components/
├── resources/
└── src/
    ├── app/
    │   ├── AppController.*
    │   ├── WindowsChrome.*
    │   └── WindowStateManager.*
    ├── serial/
    │   └── WinSerialPort.*
    └── radios/
        └── dm32uv/
            ├── DM32Connection.*
            └── DM32Constants.h
```

## Build workflow

From `native/windows/`:

```bat
SETUP.cmd
BUILD.cmd
RUN.cmd
```

Portable build output is staged under `native/windows/dist/`.

## UI foundation

The accepted visual direction at this checkpoint is an old-school service-terminal / radio-workstation GUI rather than a conventional modern dashboard.

Key visual rules:

- near-black / graphite base;
- silver and gray primary text/borders;
- amber only for prompts, active commands, warnings, and emphasis;
- green only for successful/healthy states;
- red only for errors/faults;
- monospace typography;
- ASCII YWD-PLUG identity;
- restrained CRT/terminal cues without sacrificing normal GUI controls;
- native Windows title bar behavior retained.

## Phase 2 starting point

The next phase should begin from this checkpoint and focus on the native DM-32UV **read path**:

1. Port the proven PROGRAM-mode entry sequence from the browser implementation.
2. Add safe, block-oriented memory reads.
3. Capture raw native backups without modifying the radio.
4. Compare native reads byte-for-byte against known-good browser YWD-Plug reads.
5. Build the first native decoded data model, beginning with channels.
6. Keep all write operations disabled until read/decode parity is demonstrated.

## Recovery point

A dedicated repository checkpoint branch is created from this state so later protocol work can always be compared against or rolled back to the known-good Milestone 1 foundation.
