# Contributing to YWD-Plug

YWD-Plug talks to real radios and writes persistent device configuration, so small, reviewable changes are preferred over heroic mystery commits.

## Branches

- `main` is for stable project checkpoints.
- `dev` is the active integration branch.
- Use focused feature/fix branches when a change benefits from isolated review.

## Before changing radio behavior

1. Read `YWD-Plug-ROADMAP.md`.
2. Read `docs/baseline/BASELINE.md`.
3. Preserve existing `.ywdplug` compatibility unless an explicit migration is part of the change.
4. Keep transport, UI, and radio-specific protocol concerns separated.
5. Never guess unknown firmware layouts.

## Test before merge

At minimum:

```bash
npm test -- --run
npm run build
```

For changes touching the DM-32UV path, also run `docs/baseline/DM32UV-REGRESSION.md` on hardware before declaring the milestone proven.

## DB-25D development order

DB-25D work should progress in guarded stages:

1. identification/probe
2. raw read and backup
3. decode
4. read-only editors
5. offline encode/export
6. controlled binary diff validation
7. guarded radio write
8. read-back verification

Do not collapse the dangerous half of that list into one change.

## Licensing

Preserve upstream notices. If a change adapts GPL-covered source or substantial implementation details, add the required attribution and licensing documentation in the same change. See `docs/licensing.md`.
