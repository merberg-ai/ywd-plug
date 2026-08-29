# YWD-Plug Native Windows — Phase 2 Readback

This phase starts from the proven Milestone-1 checkpoint at `checkpoint/dev-win-m1`.

## Goal

Prove the first native **read-only PROGRAM-mode path** against a real Baofeng DM-32UV / DP570UV, then use the captured image to bring up the first native codeplug decoder without implementing radio-memory writes.

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
11. Decode the channel database from a captured raw image entirely offline.

There is no native `writeMemory()` implementation in this phase.

## Hardware validation status

The first full native hardware capture has succeeded on a real DP570UV / DM-32UV.

Observed capture geometry:

- 200 blocks
- 4096 bytes per block
- 819,200 total bytes
- radio returned to normal operation after the read session
- generated binary SHA-256 matched the saved manifest
- every captured block metadata byte matched its manifest entry

The hardware capture itself is deliberately **not committed to the public repository**. It is used only as a local/private development fixture.

## Native channel decoder status

Phase 2B adds the first native C++ codeplug decoder.

The decoder follows the proven browser layout rules:

- logical channel blocks are metadata `0x12` through `0x41`
- blocks are processed in metadata order, not physical flash-address order
- metadata `0x12` stores the channel count in its first two bytes, little-endian
- first channel record begins at offset `0x10`
- first block contains 84 channel records
- following channel blocks contain up to 85 records each
- each channel record is 48 bytes
- metadata `0x42` supplies the two-byte TX-contact index records for channels 1–2047

The native decoder currently exposes read-only channel fields including:

- channel number and name
- RX/TX frequency
- RX-only/no-TX state
- analog/digital mode
- power
- bandwidth
- scan-list flags/index
- DMR color code and timeslot
- RX-group index
- TX-contact index

The decoder rejects missing or non-contiguous logical channel block sequences instead of silently decoding the wrong physical block.

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

## UI workflow

The native Connection page supports two Phase-2 workflows:

### Hardware capture

1. Connect and power on the DM-32UV.
2. Select the correct COM port.
3. Click **EXECUTE PROBE**.
4. Confirm `RADIO DETECTED` / `LINK OK`.
5. Click **RAW BACKUP**.
6. Leave the radio and cable alone until the progress reaches 100%.
7. The completed image is decoded automatically.

### Offline decoder development

1. Start YWD-Plug without touching the radio.
2. Click **LOAD LAST BACKUP**.
3. The newest `YWD-Plug-*.bin` under the backup directory is decoded locally.
4. Open **Channels** in the left navigation.

This avoids repeating a full serial dump for UI/parser work.

## Read-speed note

A complete 819,200-byte dump at 115200 baud has a hard serial-wire floor of roughly 71 seconds under 8N1 framing before protocol overhead and settling time are counted.

The full **RAW BACKUP** operation is therefore a diagnostic/safety capture, not the intended long-term normal CPS read path. A later selective read path should scan metadata and fetch only the logical blocks needed for the requested codeplug sections.

## Remaining Phase-2 validation

Before enabling any native radio-memory writes:

1. Repeat a native backup with an unchanged radio and compare hashes.
2. Capture equivalent data with browser YWD-Plug and compare logical blocks byte-for-byte.
3. Validate the native channel table against the browser decoder.
4. Build the selective/fast read path from the proven block map.
5. Port additional read-only sections (zones, scan lists, contacts, RX groups, settings).
6. Only after encoder round-trip tests are proven should a write path be considered.

## Failure data to keep

If a future read fails, preserve:

- the exact on-screen `RAW BACKUP FAILED` message
- the last percentage / block address shown
- whether the radio remained in programming mode afterward
- whether a `.bin` or `.json` file was created
- whether a normal Milestone-1 probe still succeeds after power-cycling or reconnecting the radio

The Milestone-1 checkpoint remains available at `checkpoint/dev-win-m1`.
