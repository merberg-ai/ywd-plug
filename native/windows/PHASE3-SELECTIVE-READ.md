# YWD-Plug Native Windows — Phase 3 Selective Read

Phase 3 starts from the hardware-validated Phase-2B checkpoint:

```text
checkpoint/dev-win-m2b
```

## Goal

Make **READ RADIO** the normal native CPS read path without transferring the full 800 KiB configuration image every time.

`RAW BACKUP` remains unchanged as the exhaustive diagnostic/safety capture.

## Safety boundary

Phase 3 remains read-only.

- PROGRAM mode is used only for reads.
- There is still no native write-memory API exposed to the application workflow.
- Editing and radio writes remain locked.

## Selective-read sequence

1. Open the selected Windows COM port at 115200 8N1.
2. Perform `PSEARCH` / `PASSSTA` / `SYSINFO`.
3. Query V-frame `0x01` for firmware.
4. Query V-frame `0x0A` for the radio-reported config-memory range.
5. Validate the range.
6. Enter PROGRAM mode using the already-proven read-only sequence.
7. Discover the memory map by reading only byte `0xFFF` from each 4 KiB block.
8. Locate metadata block `0x12` and read only its first 2 bytes for channel count.
9. Calculate the exact number of channel blocks needed:
   - metadata `0x12`: first 84 channels
   - following metadata blocks: 85 channels each
10. Read only the required channel blocks.
11. Read metadata `0x42` for channels 1–2047 TX-contact indexes when present.
12. Read metadata `0x43` when channel count reaches 2048+.
13. Close the radio session.
14. Decode the returned blocks directly in memory and populate the Channels workspace.

## Expected transfer for the validated 88-channel radio

The Phase-2 hardware capture reported:

- 200 total config blocks
- 88 channels
- required channel metadata: `0x12`, `0x13`
- TX-contact metadata: `0x42`

Expected payload transferred by Phase 3:

```text
metadata discovery :    200 bytes
channel count      :      2 bytes
channel block 0x12 :  4,096 bytes
channel block 0x13 :  4,096 bytes
TX contact 0x42    :  4,096 bytes
---------------------------------
total              : 12,490 bytes
```

Compare that with RAW BACKUP:

```text
819,200 bytes
```

This is about 1.5% of the raw-backup payload.

Request/response settling time will still dominate the metadata scan, so the first implementation is expected to take roughly the low-to-mid teens of seconds rather than being instantaneous.

## Hardware test

1. Pull `dev-win`.
2. Build and run normally.
3. Power on the DM-32UV and select its COM port.
4. Click **READ RADIO** directly; a separate Probe is not required.
5. Watch the status line during:
   - handshake
   - metadata-map discovery
   - channel-count read
   - selected 4 KiB data-block reads
6. On success, YWD-Plug should populate the same 88-channel table that the validated offline backup produced.

Expected final status for the known fixture is similar to:

```text
READ RADIO COMPLETE // 88 CHANNELS // 3 DATA BLOCKS // 12490 BYTES // <time>s
```

The session panel also reports:

- discovered block count
- actual data-block count
- transferred payload bytes
- elapsed time

## Validation criteria

Phase 3 is proven when:

1. READ RADIO succeeds repeatedly.
2. The decoded channel count remains 88 for the unchanged test radio.
3. Representative channels match the offline fixture, especially:
   - channel 1: `KE6CHO`
   - channel 88: `TEST Parrot`
4. DMR CC/timeslot and TX-contact indexes match the offline fixture.
5. The radio returns to normal operation after the read.
6. RAW BACKUP still works unchanged after a selective read.

## Next slice after validation

Reuse the discovered/selective block-read machinery for additional read-only CPS pages:

1. Zones
2. Scan Lists
3. RX Groups
4. Contacts / Talk Groups
5. Radio IDs
6. Radio Settings / Display

Only after the read-only native CPS is coherent and checkpointed should binary round-trip encoding and radio writes be introduced.
