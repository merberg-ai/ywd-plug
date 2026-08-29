# YWD-Plug Native Windows — Phase 2 Readback

This phase starts from the proven Milestone-1 checkpoint at `checkpoint/dev-win-m1`.

## Goal

Prove the first native **read-only PROGRAM-mode path** against a real Baofeng DM-32UV / DP570UV before porting any codeplug decoders or implementing any radio-memory write path.

## Safety boundary

Native radio-memory writes remain unavailable.

The Phase-2 path can:

1. Open the selected Windows COM port at 115200 8N1.
2. Repeat the proven `PSEARCH` / `PASSSTA` / `SYSINFO` identification handshake.
3. Query V-frame `0x01` for firmware identification.
4. Query V-frame `0x0A` for the radio-reported main config-memory range.
5. Validate that range before using it.
6. Enter PROGRAM mode using the proven browser sequence:
   - `FF FF FF FF 0C PROGRAM` -> `06`
   - `02` -> eight `FF` bytes
   - `06` -> `06`
7. Issue read commands only:
   - `52 <addr:3 little-endian> <len:2 little-endian>`
   - expect `57 <addr:3> <len:2> <data>`
8. Read the radio-reported config range in 4096-byte blocks.
9. Close the serial session.
10. Save a raw `.bin` image plus a JSON manifest.

There is no native `writeMemory()` implementation in this phase.

## Backup output

Successful backups are written under:

```text
%USERPROFILE%\Documents\YWD-Plug Backups\
```

Each capture contains:

- `YWD-Plug-<model>-<timestamp>.bin` — contiguous raw config-memory image
- `YWD-Plug-<model>-<timestamp>.json` — manifest with firmware, radio-reported range, block metadata, byte count, and SHA-256

The JSON manifest explicitly records:

```json
"writesPerformed": false
```

## Hardware test

1. Pull `dev-win` and build normally.
2. Connect and power on the DM-32UV.
3. Select the correct COM port.
4. Click **EXECUTE PROBE** first.
5. Confirm `RADIO DETECTED` / `LINK OK`.
6. Click **RAW BACKUP**.
7. Leave the radio and cable alone until the progress reaches 100%.

A full 4KB-block read at 115200 baud is intentionally not instant. The implementation includes conservative settling delays between blocks.

## Success criteria

A successful first run should end with:

- `RAW BACKUP COMPLETE`
- a `.bin` path
- a `.json` path
- a SHA-256 value
- the radio returning to normal operation after the serial session closes

Do **not** proceed to native decoding until this path is repeatable.

## Validation after the first successful capture

The next Phase-2 slice should:

1. Capture the same radio state through the existing browser YWD-Plug implementation.
2. Compare memory ranges and block metadata.
3. Compare equivalent raw 4KB blocks byte-for-byte.
4. Repeat the native backup and confirm stable hashes when the radio configuration has not changed.
5. Only then port the native channel-count/channel decoder.

## Failure data to keep

If the read fails, preserve:

- the exact on-screen `RAW BACKUP FAILED` message
- the last percentage / block address shown
- whether the radio remained in programming mode afterward
- whether a `.bin` or `.json` file was created
- whether a normal Milestone-1 probe still succeeds after power-cycling or reconnecting the radio

The Milestone-1 checkpoint remains available at `checkpoint/dev-win-m1`.