# YWD-Plug

**Browser-native radio programming and codeplug management with a modern YWD interface.**

YWD-Plug is a local-first, browser-based Channel Programming Software (CPS) project built from the open-source NeonPlug codebase. It is intended to make programming supported radios less painful, keep codeplugs understandable and portable, and grow into a capability-aware multi-radio platform without requiring a cloud account or vendor desktop CPS for routine programming.

## Project status

YWD-Plug is in early active development.

The current baseline preserves the existing YWD-Plug / NeonPlug radio functionality while the application shell is modernized to match the YWD-Hotspot design language. The **Baofeng DM-32UV / DP570UV path is the primary regression target** during this work.

The next major radio target is the **Radioddity DB-25D Pro**. DB-25D support will begin with safe identification and read-only backup/decoding. Radio writes will remain disabled until the codeplug layout, firmware family, encode/decode round trip, and binary diffs are proven.

## Core goals

- Browser-native programming using Web Serial where supported
- Local-first operation with no required account or cloud service
- Preserve existing `.ywdplug` files and import/export behavior
- Maintain `.neonplug` compatibility where practical
- Shared editors for channels, contacts, zones, scan lists, settings, and diagnostics
- Capability-aware radio support instead of one-off UI hacks
- Automatic or strongly encouraged backup before radio writes
- Safe behavior for unknown firmware and hardware variants
- Responsive desktop/mobile UI aligned with YWD-Hotspot
- Clear upstream attribution and license compliance

## Safety rules

Radio programming can modify persistent memory in real hardware. YWD-Plug development follows a few non-negotiable rules:

1. Do not break the existing DM-32UV read/write path while adding new radios.
2. Never guess unknown firmware offsets or flash layouts.
3. A write should never be the first operation on an unknown radio.
4. Back up before write and verify by reading back afterward.
5. DB-25D Pro writes stay disabled until read/decode/encode validation is complete.
6. Firmware flashing is outside the initial DB-25D implementation.

See [`YWD-Plug-ROADMAP.md`](YWD-Plug-ROADMAP.md) for the full development plan.

## Development baseline

YWD-Plug began from NeonPlug and currently tracks this frozen upstream reference for the YWD baseline:

```text
Upstream: https://github.com/infamy/NeonPlug
Commit:   8ae184770e03a93959f81c262f2ba9dcb93b0400
```

The working YWD single-file build derived from that baseline uses YWD-Plug branding and `.ywdplug` as its native codeplug extension while retaining compatibility with existing NeonPlug files.

The existing NeonPlug radio descriptor / `RadioProtocol` architecture is intentionally being preserved and extended rather than replaced with a second competing driver system.

## Local development

The source application uses React, TypeScript, Vite, Tailwind CSS, Zustand, and Vitest.

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

## Branching

- `main` — stable project checkpoints
- `dev` — active development integration branch
- feature branches — focused work when useful

Large protocol additions should be split into reviewable milestones. In particular, DB-25D detection, raw backup, decoding, offline encoding, binary validation, and guarded writing should not land as one giant change.

## Upstream and credits

YWD-Plug is derived from **NeonPlug** by `infamy` and preserves the upstream project as the foundation for its radio protocols, editors, and browser CPS architecture.

- NeonPlug: https://github.com/infamy/NeonPlug
- DM-32UV protocol specification: https://github.com/infamy/DM32-Protocol-Spec

Additional upstream projects will be credited at the point their code or substantial implementation details are incorporated. Planned DB-25D research includes RT73-related open-source work, but no such code should be copied into YWD-Plug without the required attribution and license review first.

See [`NOTICE.md`](NOTICE.md) and [`docs/licensing.md`](docs/licensing.md).

## License

The YWD-Plug baseline is distributed under the MIT License; see [`LICENSE`](LICENSE). Upstream components retain their applicable notices and licenses. GPL-derived portions, if introduced later, must remain GPL-compliant and will be documented explicitly.
