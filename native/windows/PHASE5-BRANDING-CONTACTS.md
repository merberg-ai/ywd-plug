# Phase 5 — Native Branding + Smart Referenced Contacts

Phase 5 builds forward from the hardware-validated Phase 4 sidebar state preserved at `checkpoint/dev-win-m4-sidebar`.

## Safety boundary

Phase 5 remains read-only.

- No write-memory command is exposed.
- No channel/contact encoder is invoked by the native app.
- `READ RADIO` keeps the proven single PROGRAM-mode session and DTR-reset close lifecycle.
- Contact enrichment is optional: failure to resolve the separate contact region must not invalidate the normal channel/zone/scan/RX-group read.
- `RAW BACKUP` remains the exhaustive config-region diagnostic capture.

## Branding assets

The Windows build now ships the supplied YWD-Plug branding:

- `resources/ywd-plug-win.ico` is compiled into `YWD-Plug.exe` as the native Windows executable icon and is also set as the Qt application/window icon.
- `resources/ywd-plug-win-logo1.png` is an application-optimized 600x450 copy of the supplied artwork.
- The artwork is used as the hero panel in the startup POST/splash screen.
- A compact cropped treatment is used in the main masthead so the branding does not consume the working area.

The native resource script is configured by CMake with an absolute icon path so MSVC's resource compiler does not depend on its current working directory.

## Smart contact read

The DM-32UV digital contact database is separate from the 4KB config-block region, so Phase 5 intentionally does **not** bulk-read the entire contact database.

The live `READ RADIO` sequence now:

1. performs the established PSEARCH / PASSSTA / SYSINFO handshake;
2. queries V-frame `0x0A` for the config memory range;
3. queries V-frame `0x0F` for the contact-memory range;
4. queries V-frame `0x10` for contact capacity when available;
5. enters PROGRAM mode once;
6. performs the established config metadata scan and selective codeplug reads;
7. examines TX-contact mapping blocks `0x42` / `0x43` and extracts the unique contact indexes referenced by loaded channels;
8. reads the four-byte contact database count header;
9. calculates only the 4KB contact pages required by those referenced indexes;
10. reads those pages and decodes the referenced contact records;
11. closes the same radio session using the proven DTR-reset/reopen lifecycle.

### Contact record layout

The native decoder follows the existing browser driver's 92-byte (`0x5C`) record layout:

- `0x00-0x0F` name, 16 ASCII bytes;
- `0x10-0x13` DMR ID, uint32 little-endian;
- `0x14-0x1B` callsign, 8 bytes;
- `0x1C-0x2B` city, 16 bytes;
- `0x2C-0x3B` province/state, 16 bytes;
- `0x3C-0x4B` country, 16 bytes;
- `0x4C-0x5B` remark, 16 bytes.

The first aligned contact page contains a 16-byte header followed by 44 contacts. Later pages contain 44 records beginning at page offset zero.

## UI

`05 Contacts` becomes available after a successful live read when referenced contacts are resolved.

The Contacts page displays:

- contact index;
- contact name;
- DMR ID;
- callsign;
- location fields;
- which loaded channels reference the contact.

The Channels page also replaces the bare TX contact index with the resolved contact name when available.

An offline raw config backup can still populate Channels/Zones/Scan Lists/RX Groups, but it cannot populate Contacts because the contact database lives outside that raw config capture.

## First hardware test

1. Build and launch YWD-Plug.
2. Confirm the supplied icon appears on the EXE/taskbar/title bar.
3. Confirm the new artwork splash renders and hands off normally.
4. Confirm the compact masthead artwork does not distort the working layout.
5. Select the DM-32UV COM port and run `READ RADIO`.
6. Verify the established channel/zone/scan/RX-group counts still match the Phase 4 baseline.
7. Verify `05 Contacts` becomes enabled when referenced contacts resolve.
8. Open Contacts and compare a few resolved names/DMR IDs against the source radio/browser codeplug.
9. Open Channels and verify digital channels now show resolved TX-contact names.
10. Run Probe / Read Radio again to ensure repeated-session behavior remains stable.

Do not enable radio writes until contact and codeplug readback have been hardware-validated and the later encoder/round-trip phase has its own protected checkpoint.
