# Phase 4 — Read-Only Codeplug Structures

Phase 4 expands the native Windows selective-read path beyond channels while preserving the proven serial/session lifecycle from `checkpoint/dev-win-m3-serial`.

## Safety boundary

This phase is still strictly read-only.

- No write-memory API is enabled.
- No channel, zone, scan-list, RX-group, or contact encoder is invoked by the native application.
- `RAW BACKUP` remains the exhaustive safety/diagnostic capture path.
- `READ RADIO` remains the normal selective path.

## Data added to the selective read

After the single config-block metadata scan, the native reader now fetches:

- the channel blocks required by the channel count (`0x12+`),
- TX-contact index block `0x42` and `0x43` when required,
- scan-list block `0x11`,
- RX-group block `0x0F`,
- zone blocks `0x5C` through `0x64` when present.

All of these blocks are fetched during the same PROGRAM-mode serial session. The reader does not disconnect and reconnect between codeplug sections.

## Native decoders

`DM32CodeplugDecoder` ports the browser driver's established layouts for the read-only structures used in this phase.

### Zones

- Metadata `0x5C` through `0x64`.
- 145 bytes per zone.
- First zone block has a 16-byte header; subsequent zone blocks start their first zone at offset zero.
- Zone name is the first 11 bytes.
- Channel count is byte 16.
- Channel membership starts at byte 17 as little-endian 16-bit channel numbers.

### Scan lists

- Metadata `0x11`.
- Count at offset zero.
- 57 bytes per list.
- Name, hang time, priority policy, designated TX policy, and up to 15 channel members are decoded.

### RX groups

- Metadata `0x0F`.
- Active-group bitmask at offsets `0x00-0x03`.
- Entries start at `0x11` and are 109 bytes each.
- Group name and the 3-byte little-endian DMR/Talk Group member IDs are decoded.

## UI

The existing channel workspace is promoted to a read-only `CODEPLUG DATABASE` with four terminal-style views:

1. Channels
2. Zones
3. Scan Lists
4. RX Groups

`LOAD BACKUP` runs the same structure decoders against an existing native raw `.bin` capture, so UI/decoder testing does not require another radio read.

## Contacts

The large digital contact database is intentionally not bulk-loaded in this phase. It lives in the separate V-frame-backed contact memory region and can be much larger than the small config structures above. A later phase should read only the contact records needed by the loaded codeplug first, then add an explicit full-contact-database operation if useful.

## Hardware test order

1. Build and start YWD-Plug.
2. Use `LOAD BACKUP` first and verify Channels/Zones/Scan Lists/RX Groups populate.
3. Select the DM-32UV COM port and run `READ RADIO`.
4. Verify all four views match the offline backup.
5. Run another Probe and another Read Radio to confirm the proven DTR-reset/reopen lifecycle remains stable.

Do not enable native radio writes until the read-only structures have been compared against the browser implementation and round-trip encoding has its own protected test phase.
