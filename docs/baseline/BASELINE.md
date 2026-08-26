# YWD-Plug Baseline Checkpoint

## Purpose

This document freezes the starting assumptions for active YWD-Plug development so UI work and DB-25D research do not silently change the already-working radio behavior.

## Upstream source reference

```text
Repository: https://github.com/infamy/NeonPlug
Commit:     8ae184770e03a93959f81c262f2ba9dcb93b0400
```

YWD-Plug's existing standalone build was derived from this source line and adds YWD branding/namespace behavior and `.ywdplug` codeplug naming.

## Existing compatibility that must be preserved

- browser radio connection workflow
- current DM-32UV / DP570UV read path
- current DM-32UV / DP570UV write path
- channels, contacts, zones, scan lists, digital settings, radio settings, and diagnostics already supported by the baseline
- `.ywdplug` import/export
- compatibility import of `.neonplug` files
- local snapshots/backups and diagnostics already present in the YWD build
- radio-to-radio conversion behavior already present in the baseline

## Architecture decision

Current NeonPlug already provides a radio descriptor / `RadioProtocol` architecture and capability-aware UI. YWD-Plug will extend that architecture rather than creating a second parallel driver API solely to match early roadmap pseudocode.

The DB-25D Pro should therefore enter the source tree as a normal radio implementation/descriptor with its own capabilities and protocol/layout code.

## UI-only protection boundary

During the initial YWD-Hotspot-style UI refresh, avoid changing radio protocol, serialization, store semantics, validation semantics, or write behavior. Visual work should be independently reviewable from radio behavior changes.

## Hardware authority

Automated tests are necessary but do not replace hardware regression. A real DM-32UV read/write/read-back test remains the authority before declaring a radio-affecting milestone good.
