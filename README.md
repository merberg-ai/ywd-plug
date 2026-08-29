# YWD-Plug

**Radio programming and codeplug management with a modern YWD interface.**

YWD-Plug is a local-first Channel Programming Software (CPS) project built from the open-source NeonPlug codebase. The original application is browser-based; a true native Windows application is now being developed alongside it using C++20 and Qt 6/QML.

## Project status

YWD-Plug is in early active development.

The current browser baseline preserves the existing YWD-Plug / NeonPlug radio functionality while the application shell is modernized to match the YWD-Hotspot design language. The **Baofeng DM-32UV / DP570UV path is the primary regression target** during this work.

The next major radio target is the **Radioddity DB-25D Pro**. DB-25D support will begin with safe identification and read-only backup/decoding. Radio writes will remain disabled until the codeplug layout, firmware family, encode/decode round trip, and binary diffs are proven.

### Native Windows application

The `dev-win` branch contains a separate native Windows CPS under [`native/windows/`](native/windows/). It uses **C++20 + Qt 6/QML + Qt SerialPort** rather than embedding the browser application.

The first native milestone is intentionally read-only: enumerate Windows COM ports and perform the proven DM-32UV `PSEARCH` → `PASSSTA` → `SYSINFO` identification handshake. It does **not** enter programming mode and does **not** write radio memory yet.

From `native/windows/` on Windows:

```bat
SETUP.cmd
BUILD.cmd
RUN.cmd
```

The native toolchain also includes a colorized PowerShell setup/build/install launcher and a current-user installer path. See [`native/windows/README.md`](native/windows/README.md).

## Core goals

- Browser-native programming using Web Serial where supported
- A true native Windows CPS using Qt/C++
- Local-first operation with no required account or cloud service
- Preserve existing `.ywdplug` files and import/export behavior
- Maintain `.neonplug` compatibility where practical
- Shared editors for channels, contacts, zones, scan lists, settings, and diagnostics
- Capability-aware radio support instead of one-off UI hacks
- Automatic or strongly encouraged backup before radio writes
- Safe behavior for unknown firmware and hardware variants
- Responsive UI aligned with YWD-Hotspot / KJ6YWD design language
- Clear upstream attribution and license compliance

## Safety rules

Radio programming can modify persistent memory in real hardware. YWD-Plug development follows a few non-negotiable rules:

1. Do not break the existing DM-32UV read/write path while adding new radios.
2. Never guess unknown firmware offsets or flash layouts.
3. A write should never be the first operation on an unknown radio.
4. Back up before write and verify by reading back afterward.
5. DB-25D Pro writes stay disabled until read/decode/encode validation is complete.
6. Firmware flashing is outside the initial DB-25D implementation.
7. Native Windows writes stay disabled until the native read path is validated against the existing browser implementation.

See [`YWD-Plug-ROADMAP.md`](YWD-Plug-ROADMAP.md) for the full development plan.

## Development baseline

YWD-Plug began from NeonPlug and currently tracks this frozen upstream reference for the YWD baseline:

```text
Upstream: https://github.com/infamy/NeonPlug
Commit:   8ae184770e03a93959f81c262f2ba9dcb93b0400
```

The working YWD single-file build derived from that baseline uses YWD-Plug branding and `.ywdplug` as its native codeplug extension while retaining compatibility with existing NeonPlug files.

The existing NeonPlug radio descriptor / `RadioProtocol` architecture is intentionally being preserved and extended rather than replaced with a second competing driver system.

## Browser development

The source browser application uses React, TypeScript, Vite, Tailwind CSS, Zustand, and Vitest.

Typical development commands:

```bash
npm ci
npm run dev
```

Run the regression suite and production build before merging behavior changes:

```bash
npm test -- --run
npm run build
```

Build the standalone single-file version with:

```bash
npm run build:single
```

Web Serial normally requires a Chromium-based browser and a secure context such as HTTPS or localhost. The standalone build is also intended to remain useful for local/offline operation where browser support permits it.

## Codeplug files

`.ywdplug` is YWD-Plug's native codeplug extension. The current format is a ZIP archive containing `codeplug.json` plus any format-specific data required by the application.

Existing `.neonplug` files remain an important compatibility path. Serialization changes must not be slipped into unrelated UI work; schema migrations must be explicit and tested.

The native Windows application is intended to use the same `.ywdplug` format rather than inventing a Windows-only codeplug format.

## Branching

- `main` — stable project checkpoints
- `dev` — active browser/multi-radio development integration branch
- `dev-win` — native Windows Qt/C++ development
- feature branches — focused work when useful

Large protocol additions should be split into reviewable milestones. In particular, DB-25D detection, raw backup, decoding, offline encoding, binary validation, and guarded writing should not land as one giant change. The native Windows port follows the same rule: identify, read, compare, encode offline, then enable guarded writes.

## Upstream and credits

YWD-Plug is derived from **NeonPlug** by `infamy` and preserves the upstream project as the foundation for its radio protocols, editors, and browser CPS architecture.

- NeonPlug: https://github.com/infamy/NeonPlug
- DM-32UV protocol specification: https://github.com/infamy/DM32-Protocol-Spec

Additional upstream projects will be credited at the point their code or substantial implementation details are incorporated. Planned DB-25D research includes RT73-related open-source work, but no such code should be copied into YWD-Plug without the required attribution and license review first.

See [`NOTICE.md`](NOTICE.md) and [`docs/licensing.md`](docs/licensing.md).

## License

The YWD-Plug baseline is distributed under the MIT License; see [`LICENSE`](LICENSE). Upstream components retain their applicable notices and licenses. GPL-derived portions, if introduced later, must remain GPL-compliant and will be documented explicitly.
