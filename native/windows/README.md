# YWD-Plug Native Windows

Native Windows CPS for YWD-Plug, built with **C++20 + Qt 6/QML** on the `dev-win` branch.

This is a real native application. It does not embed the browser YWD-Plug UI and it does not require Chromium/Web Serial at runtime.

## Current phase

Major recovery checkpoints include:

```text
checkpoint/dev-win-m1
checkpoint/dev-win-m2b
checkpoint/dev-win-m3-serial
checkpoint/dev-win-m4-sidebar
```

`dev-win` is now in **Phase 5: native branding + smart referenced Contacts**.

The current read-only native CPS includes:

1. Native `PSEARCH` / `PASSSTA` / `SYSINFO` handshake.
2. Proven DTR-reset / 400 ms reopen lifecycle for repeated radio sessions.
3. PROGRAM-mode entry using the browser-proven read-only sequence.
4. Fast metadata discovery across the 4 KiB config-block map.
5. Selective live reads for Channels, Zones, Scan Lists, and RX Groups.
6. Native C++ decoders and dedicated sidebar workspaces for those codeplug sections.
7. Smart contact-memory discovery using contact V-frames `0x0F` / `0x10`.
8. Selective loading of only contact pages referenced by channel TX-contact mappings.
9. Channel TX-contact name resolution after a successful live read.
10. A dedicated referenced Contacts workspace.
11. The supplied YWD-Plug Windows icon compiled into the native executable.
12. The supplied YWD-Plug artwork integrated into the startup POST/splash and compact main masthead.
13. The exhaustive **RAW BACKUP** path, SHA-256 manifest, and offline **LOAD BACKUP** workflow.

**Native radio-memory writes remain unavailable and editing remains locked.**

See:

- [`PHASE2-READBACK.md`](PHASE2-READBACK.md) — exhaustive raw backup validation
- [`PHASE3-SELECTIVE-READ.md`](PHASE3-SELECTIVE-READ.md) — fast/selective READ RADIO validation
- [`PHASE4-CODEPLUG-STRUCTURES.md`](PHASE4-CODEPLUG-STRUCTURES.md) — Channels/Zones/Scan Lists/RX Groups
- [`PHASE5-BRANDING-CONTACTS.md`](PHASE5-BRANDING-CONTACTS.md) — Windows branding + selective referenced Contacts

## Stack

- C++20
- Qt 6.8+ Core / GUI / Quick / Quick Controls / Concurrent
- Qt Quick / QML
- Native Win32 COM transport (`CreateFile`, `SetCommState`, `ReadFile`, `WriteFile`)
- Native Windows executable resources (`.rc` / `.ico`)
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

For the normal Phase-5 radio test:

1. Confirm the supplied YWD-Plug icon is used by the native executable/window/taskbar.
2. Confirm the branded POST splash appears and hands off normally.
3. Select the correct COM port.
4. Click **READ RADIO** directly; Probe is optional.
5. Verify Channels, Zones, Scan Lists, and RX Groups still match the known codeplug.
6. If referenced digital contacts resolve, **Contacts** becomes available in the sidebar.
7. Open Contacts and compare a few names/DMR IDs with the source radio/browser codeplug.
8. Open Channels and verify TX-contact names replace bare indexes where available.
9. Run another Probe / READ RADIO cycle to confirm repeated-session behavior remains stable.

Contact enrichment is deliberately optional. A firmware/contact-region incompatibility should produce a contact warning while leaving the core codeplug read usable.

## Raw backups

**RAW BACKUP** remains the exhaustive safety/diagnostic operation for the radio's config-block region. Successful raw backups are stored under:

```text
%USERPROFILE%\Documents\YWD-Plug Backups\
```

Each raw backup creates a `.bin` image and a `.json` manifest.

**LOAD BACKUP** decodes the newest saved config image without touching the radio. Because the digital contact database lives in a separate memory region, an existing raw config backup can populate Channels/Zones/Scan Lists/RX Groups but **does not contain the separate contact database**. Contacts therefore require a live `READ RADIO` session in Phase 5.

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
├── PHASE4-CODEPLUG-STRUCTURES.md
├── PHASE5-BRANDING-CONTACTS.md
├── qml/
│   ├── ConnectionPage.qml
│   ├── ChannelsPage.qml
│   ├── ZonesPage.qml
│   ├── ScanListsPage.qml
│   ├── ContactsPage.qml
│   ├── RXGroupsPage.qml
│   └── SplashWindow.qml
├── resources/
│   ├── ywd-plug-win.ico
│   ├── ywd-plug-win-logo1.png
│   └── ywd-plug-win.rc
└── src/
    ├── app/
    ├── serial/
    │   └── WinSerialPort.*
    └── radios/
        └── dm32uv/
            ├── DM32Connection.*
            ├── DM32SelectiveRead.cpp
            ├── DM32MemoryBlock.h
            ├── DM32ContactBlock.h
            ├── DM32ChannelDecoder.*
            ├── DM32CodeplugDecoder.*
            └── DM32ContactDecoder.*
```

## Next validation gate

Phase 5 must now be compiled and hardware-tested on the Windows/DM-32UV setup.

After branding and referenced-contact readback are validated:

1. add read-only Radio IDs;
2. add Settings / Display structures;
3. complete the native read-only CPS surface;
4. checkpoint the complete read-only state;
5. add native encoders and prove binary round-trip equality **without writing the radio**;
6. only after protected round-trip validation, design the later radio-write milestone.

Radio-memory writes remain a later milestone.
