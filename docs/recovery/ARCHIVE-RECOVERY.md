# YWD-Plug source recovery

## Recovered artifact

The only surviving application package available during repository initialization was:

`kj6ywd-plug-0.0.3-alpha1.zip`

It contains a complete working deployment, but the application itself is bundled into a single production `index.html`. The original TypeScript/React module boundaries are not present in the archive.

The archive is therefore a reference build, not the canonical source tree.

## What the archive proved

Inspection of the bundle showed the same radio-management engine and architecture as NeonPlug's `8ae184770e03a93959f81c262f2ba9dcb93b0400` checkpoint, including the channel virtualizer introduced by that commit. That commit had already been selected independently as the YWD-Plug recovery baseline.

The archive also preserves evidence of the earlier YWD/KJ6YWD work:

- Web Serial radio programming remains bundled.
- DM-32UV/DP570UV support is present.
- codeplug ZIP payload is still `codeplug.json`.
- legacy `.neonplug` import is supported.
- the later KJ6YWD deployment used `.kj6plug` for new exports and still imported `.ywdplug`/`.neonplug`.
- browser storage was deliberately namespaced away from upstream NeonPlug.
- UI branding/theme was layered over the app rather than implemented by rewriting the radio protocol.

## Canonical recovery decision

The maintained project returns to the **YWD-Plug** product name and uses **`.ywdplug` as its native codeplug extension**. Legacy `.neonplug` remains readable. `.kj6plug` can be retained as an import-compatibility alias after the recovered TypeScript baseline passes hardware regression.

The source is rebuilt from the exact upstream commit and then patched only at the application identity, storage, export/import, and presentation layers. `src/radios/dm32uv/` is hash-checked before and after recovery and must remain byte-identical during this stage.

## Recovery gate

The automated recovery runs:

```bash
npm ci
npm test -- --run
npm run build
npm run build:single
```

A source recovery commit is allowed only when all gates pass.
