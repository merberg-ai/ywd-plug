# YWD-Plug Project Roadmap
## UI Modernization + Radioddity DB-25D Pro Support

**Project:** YWD-Plug  
**Primary goal:** Evolve the existing browser-native CPS into a multi-radio programming platform while preserving all current working functionality.  
**New radio target:** Radioddity DB-25D Pro  
**Existing radio target:** Baofeng DM-32UV / current YWD-Plug functionality  
**UI direction:** Bring YWD-Plug much closer to the current YWD-Hotspot visual language, layout, colors, controls, and overall polish.  
**Licensing direction:** GPL-compatible where GPL-derived DB25-D/RT73 code is incorporated, with clear attribution and preservation of upstream notices.

---

# 1. Non-Negotiable Project Rules

These rules should guide every change.

1. **Do not break the existing YWD-Plug radio path.**
   - Current DM-32UV connect/read/write/import/export functionality must continue working throughout development.
   - Existing `.ywdplug` behavior must remain compatible unless a migration is explicitly added.
   - UI cleanup must not be allowed to silently alter radio data.

2. **Separate radio-specific protocol code from the common UI.**
   - YWD-Plug should become a multi-radio CPS, not a collection of one-off hacks.
   - The browser transport, diagnostics, common editors, validation system, import/export system, and UI shell should be reusable.
   - Radio-specific behavior belongs behind a driver/adapter layer.

3. **Unknown firmware means safe behavior.**
   - A DB-25D Pro with an unrecognized firmware/hardware family may be probed and read if safe.
   - Writing should be disabled until the codeplug layout is known and validated.
   - Never guess at firmware offsets or flash layouts.

4. **Back up before write.**
   - Every supported radio write flow should strongly encourage or automatically create a backup of the last successful radio read.
   - A write should never be the first operation performed on an unknown radio.

5. **Firmware flashing is out of scope initially.**
   - YWD-Plug should program codeplugs, not update DB-25D firmware in the first implementation.
   - Radioddity's official IAP updater remains the recommended firmware-update path until hardware variants and recovery behavior are thoroughly understood.

6. **GPL compliance is part of the architecture, not cleanup work.**
   - Preserve upstream copyright notices.
   - Add attribution before importing derived source.
   - Document which files/algorithms are adapted from GPL projects.
   - Make the repository license compatible with any GPLv3-derived implementation included in the distributed project.

---

# 2. Desired End State

YWD-Plug should become a browser-native radio management environment with a shared shell and per-radio capabilities.

Conceptually:

```text
YWD-Plug
│
├── Application Shell
│   ├── Dashboard / Home
│   ├── Radio
│   ├── Channels
│   ├── Contacts
│   ├── Zones
│   ├── Scan Lists
│   ├── APRS
│   ├── Settings
│   ├── Diagnostics
│   ├── Import / Export
│   └── About / Credits
│
├── Browser Services
│   ├── Web Serial
│   ├── File import/export
│   ├── Local backup storage
│   ├── Logging
│   └── Validation
│
├── Radio Driver API
│   ├── DM32UVDriver
│   └── DB25DProDriver
│
└── Radio Codecs
    ├── DM32UV codeplug codec
    └── DB25-D Pro codeplug codec
```

The UI should expose only the features supported by the currently connected or loaded radio.

---

# 3. Phase 0 — Baseline and Safety Checkpoint

Before redesigning anything, establish a reproducible known-good baseline.

## Tasks

- Record the current branch, commit, and repository status.
- Run the current YWD-Plug exactly as users do today.
- Verify current Web Serial connection behavior.
- Verify current DM-32UV read.
- Verify current DM-32UV write if a safe test radio/codeplug is available.
- Verify current import.
- Verify current export to `.ywdplug`.
- Verify serial logging/diagnostics.
- Capture screenshots of the current working UI.
- Save one known-good exported `.ywdplug` test fixture.
- If possible, save one representative serial/debug log from a successful radio session.

## Deliverables

```text
docs/baseline/
  current-ui/
  dm32uv-read-notes.md
  dm32uv-write-notes.md
  known-good-fixtures.md
```

Recommended test fixtures:

```text
test/fixtures/
  dm32uv-known-good.ywdplug
  dm32uv-minimal.ywdplug
```

## Exit criteria

Do not proceed until the current application can be restored and retested from this checkpoint.

---

# 4. Phase 1 — Establish the YWD-Plug Design System

Do the visual cleanup before introducing a second radio workflow, but avoid major application-state refactors in the same commits.

The goal is to make YWD-Plug feel like it belongs beside YWD-Hotspot.

## Visual direction

Carry over the visual language of YWD-Hotspot:

- dark background hierarchy
- cyan/blue primary accents
- restrained magenta accent where appropriate
- clean card layouts
- consistent rounded containers
- modern toggle switches instead of raw browser checkboxes
- consistent status badges
- clear locked/disabled states
- responsive mobile-first layout
- high-contrast text
- compact but readable controls
- polished navigation
- consistent spacing scale
- consistent button hierarchy
- clear destructive-action styling
- subtle RF/tech identity without becoming visually noisy

## Build shared UI primitives

Create reusable components/classes for:

```text
button
button-primary
button-secondary
button-danger
card
section
status-badge
toggle
text-input
select
number-input
table
toolbar
modal
toast
progress
empty-state
warning-banner
radio-status
nav-item
```

## CSS/token layer

Create design tokens rather than repeating hard-coded values.

Example structure:

```css
:root {
  --bg-app: ...;
  --bg-panel: ...;
  --bg-elevated: ...;

  --text-primary: ...;
  --text-secondary: ...;
  --text-muted: ...;

  --accent-primary: ...;
  --accent-secondary: ...;
  --accent-danger: ...;
  --accent-warning: ...;
  --accent-success: ...;

  --border-default: ...;
  --radius-sm: ...;
  --radius-md: ...;
  --radius-lg: ...;

  --space-1: ...;
  --space-2: ...;
  --space-3: ...;
}
```

Where practical, derive actual values from the current YWD-Hotspot CSS instead of visually approximating them.

## Layout proposal

Desktop:

```text
┌───────────────┬────────────────────────────────────┐
│ YWD-PLUG      │ top status / radio summary         │
│               ├────────────────────────────────────┤
│ Dashboard     │                                    │
│ Radio         │ current editor                     │
│ Channels      │                                    │
│ Contacts      │                                    │
│ Zones         │                                    │
│ Scan Lists    │                                    │
│ APRS          │                                    │
│ Diagnostics   │                                    │
│ Settings      │                                    │
│ About         │                                    │
└───────────────┴────────────────────────────────────┘
```

Mobile:

- compact top header
- drawer or compact navigation
- stacked cards
- scrollable tables/editors
- no desktop-only controls
- buttons large enough for touch
- prevent horizontal overflow where possible

## Preserve behavior

During this phase:

- do not change serialization
- do not change Web Serial protocol code
- do not change radio write logic
- do not change codeplug semantics

## Exit criteria

The same DM-32UV regression checklist from Phase 0 passes with the redesigned interface.

---

# 5. Phase 2 — Refactor to a Radio Driver Architecture

This is the architectural prerequisite for DB-25D Pro support.

## Define a driver contract

A radio driver should expose something close to:

```js
class RadioDriver {
  static id;
  static displayName;

  async probe(transport) {}
  async identify(transport) {}

  async readRadio(transport, progress) {}
  async writeRadio(transport, model, progress) {}

  decode(rawBytes) {}
  encode(model) {}

  validate(model) {}

  getCapabilities() {}
  getFirmwareInfo() {}
}
```

Do not force every radio into identical capabilities.

Instead expose a capability structure:

```js
{
  channels: true,
  zones: true,
  scanLists: true,
  dmrContacts: true,
  rxGroups: true,
  aprsAnalog: true,
  aprsDmr: true,
  gps: true,
  firmwareUpdate: false
}
```

## Transport abstraction

Keep Web Serial separate from the driver.

Suggested interface:

```js
transport.open(options)
transport.close()
transport.write(bytes)
transport.readExact(length)
transport.readUntil(...)
transport.flush()
transport.setSignals(...)
transport.onLog(...)
```

This gives both radio drivers the same transport without sharing protocol assumptions.

## Move current DM-32UV code behind the driver contract

This is an important intermediate checkpoint.

Before adding DB-25D:

- create `DM32UVDriver`
- move existing identification/read/write logic into it
- keep the existing UI operational through the new interface
- verify no feature regression

## Exit criteria

YWD-Plug behaves identically for the DM-32UV, but the UI no longer directly owns radio-protocol logic.

---

# 6. Phase 3 — Licensing and Upstream Attribution

Do this before importing GPL-derived implementation code.

## Upstream projects to review and credit

Primary known groundwork:

- David Pye / M0DMP `RT73-utils`
- MM7DBT CPEditor where relevant
- other upstream sources only if code or substantial implementation details are actually incorporated

## Repository additions

Recommended:

```text
LICENSE
NOTICE.md
docs/licensing.md
docs/upstream/db25d-protocol.md
```

`NOTICE.md` should clearly explain:

- YWD-Plug project ownership
- applicable project license
- upstream project names
- upstream authors
- upstream repository URLs
- what portions were adapted
- original license
- any modifications made by YWD-Plug

Do not remove original source headers from adapted files.

## Source organization

Keep derived code visibly attributable, for example:

```text
src/radios/db25d/
  README.md
  driver.js
  protocol.js
  codec.js
  layouts/
```

The local `README.md` can identify which pieces originated from or were informed by which upstream implementation.

## Exit criteria

Licensing is clear before derived DB25-D code lands in the main implementation.

---

# 7. Phase 4 — DB-25D Pro Probe and Diagnostics Only

The first DB-25D Pro milestone should not write anything.

## Initial supported actions

- select DB-25D Pro driver
- request Web Serial device
- open port at the appropriate baud rate
- send identification/probe request
- capture raw bytes
- recognize valid DB25-D-family response
- display firmware string
- display hardware/batch identifier where available
- display transfer/protocol metadata
- export diagnostic log

## UI example

```text
Connected Radio

Model family: Radioddity DB25-D
Variant: DB25-D Pro
Firmware: 909E....
Hardware family: DBB / D6T / D66 / ...
Serial port: Connected
Baud: 115200

Protocol status: Recognized
Codeplug read support: Experimental
Codeplug write support: Disabled
```

## Diagnostics logging

Log:

- timestamp
- TX command
- RX response length
- response summary
- parser result
- timeout
- unexpected packet
- reconnect event

Provide a way to save the log without leaking unrelated browser data.

## Exit criteria

Repeated connect/disconnect/probe cycles work without wedging the radio or browser session.

---

# 8. Phase 5 — DB-25D Pro Raw Codeplug Read

Next goal: obtain a complete raw backup from the radio.

## Implement

- enter/read protocol handshake
- discover page/block count
- read blocks
- verify expected lengths
- track progress
- handle retryable read errors
- concatenate final raw image
- calculate checksum/hash
- save binary backup

Example file:

```text
db25d-pro-backup-YYYYMMDD-HHMMSS.bin
```

## Safety

At this stage:

**NO WRITE COMMANDS.**

Even if upstream code contains write support, leave write disabled until the Pro layout has been verified.

## Add backup metadata

Example:

```json
{
  "radio": "Radioddity DB-25D Pro",
  "firmware": "...",
  "hardwareFamily": "...",
  "readAt": "...",
  "sha256": "...",
  "size": 123456
}
```

## Exit criteria

Multiple reads from an unchanged radio produce identical or explainably different images.

If volatile bytes change, document them.

---

# 9. Phase 6 — Port and Validate the DB25-D Codeplug Codec

Now adapt the GPL codeplug parsing work.

## Initial decode targets

Start with low-risk, easy-to-verify sections:

1. device information
2. basic settings
3. channels
4. zones
5. contacts
6. RX groups
7. scan lists
8. button configuration
9. APRS
10. GPS-related configuration
11. DMR-specific settings

## Golden comparison method

For every decoded field:

1. read radio with YWD-Plug
2. open/read same radio with Radioddity CPS
3. open/read same radio with CPEditor
4. compare values

Change **one field at a time** in official software and perform another read.

Example:

```text
Baseline binary
       ↓
change APRS frequency only
       ↓
new binary
       ↓
binary diff
       ↓
confirm mapped field
```

This is particularly important for newer Pro firmware fields.

## Unknown bytes

Never overwrite bytes simply because they are unknown.

Preserve untouched regions exactly through decode/edit/encode.

A safe representation may contain:

```js
{
  parsed: {...},
  preservedRawSections: {...}
}
```

## Exit criteria

A read → decode → encode cycle produces a binary image that is identical to the original for all regions that should remain unchanged.

---

# 10. Phase 7 — Common Radio Data Model

Once the DB25-D is decoding reliably, introduce a shared semantic model where appropriate.

Example:

```js
{
  identity: {},
  general: {},
  channels: [],
  zones: [],
  scanLists: [],
  contacts: [],
  rxGroups: [],
  aprs: {},
  buttons: {},
  metadata: {}
}
```

Each radio driver converts between:

```text
radio-native data
        ↕
common YWD-Plug model
```

Do not force unsupported settings into fake defaults.

Use capability-driven UI visibility.

Example:

```text
DB25-D Pro
  APRS tab: visible

DM-32UV
  Analog APRS controls: hidden if unsupported
```

---

# 11. Phase 8 — DB-25D Pro Read-Only Editor UI

Before writing, expose decoded data in the UI.

## Pages

### Radio

- model
- firmware
- hardware/batch
- DMR radio ID
- callsign
- general settings
- button assignments

### Channels

Columns should include relevant mode-aware fields:

```text
Name
Mode
RX
TX
Power
Scan
Contact
RX Group
Color Code
Slot
DMR Mode
APRS
```

Hide irrelevant fields for analog channels.

### Contacts

- DMR contacts
- talkgroups
- private calls
- group calls

### RX Groups

Provide drag/add/remove editing.

### Zones

Provide simple channel membership management.

### Scan Lists

Show priority and membership clearly.

### APRS

Make this substantially nicer than vendor CPS.

Analog section:

```text
Enabled
Frequency
Callsign
SSID
Path
Symbol
Position source
Beacon interval
Power
PTT upload
```

Digital APRS section should be separate and clearly explained.

## Exit criteria

Every important value in the DB25-D Pro codeplug can be inspected in the browser and matches official software.

---

# 12. Phase 9 — Safe Editing Without Radio Write

Allow changes in memory but still do not transmit them to the radio.

## Validation

Build a central validator for:

- duplicate channel names where disallowed/problematic
- invalid frequencies
- invalid color codes
- invalid time slots
- missing contacts
- missing RX groups
- broken zone references
- broken scan-list references
- unsupported APRS combinations
- index overflow
- maximum collection sizes
- invalid DMR ID values
- unknown firmware restrictions

## Binary generation

Allow:

**Export modified binary**

but not yet:

**Write Radio**

Compare generated images against CPS/CPEditor-created equivalents.

## Exit criteria

Known controlled edits produce expected binary diffs only.

---

# 13. Phase 10 — DB-25D Pro Write Support

Only enable this after read/decode/encode validation is mature.

## Mandatory write gate

Before writing:

```text
✓ Supported model
✓ Supported firmware family
✓ Supported hardware family
✓ Successful read in current session
✓ Backup exists
✓ Codeplug validates
✓ Encoded size correct
✓ Internal references valid
✓ No unexpected layout changes
```

If any item fails:

**WRITE DISABLED**

## Write workflow

```text
Read radio
   ↓
automatic backup
   ↓
edit
   ↓
validate
   ↓
encode
   ↓
write blocks
   ↓
verify if protocol permits
   ↓
radio restart/reload
   ↓
read back
   ↓
compare key fields
```

## Recovery behavior

If writing fails:

- stop immediately
- preserve diagnostic log
- do not retry endlessly
- clearly tell the user whether a retry is safe
- retain the pre-write binary backup

## Exit criteria

Multiple controlled write/read-back cycles succeed on the actual DB-25D Pro.

---

# 14. Phase 11 — Improve `.ywdplug` for Multiple Radios

The file format should become model-aware while preserving current compatibility.

Suggested wrapper:

```json
{
  "format": "ywdplug",
  "schema": 2,
  "radio": {
    "driver": "radioddity-db25d-pro",
    "model": "Radioddity DB-25D Pro",
    "firmwareFamily": "..."
  },
  "data": {}
}
```

For old files:

```text
schema 1
   ↓
existing DM-32UV loader
```

Do not make existing exports unreadable.

## Consider two backup types

### Portable logical backup

```text
my-radio.ywdplug
```

Human-readable/portable data.

### Exact radio image

```text
my-radio.bin
```

Exact binary recovery/reference image.

---

# 15. Phase 12 — Import Helpers and Quality-of-Life Tools

Once both radios are stable, add higher-level tooling.

## Repeater import

Support structured imports from:

- CSV
- RepeaterBook exports where licensing/terms permit
- user-maintained repeater lists

Allow users to preview changes before import.

## DMR talkgroup helper

Provide reusable talkgroup/contact sets.

Example:

```text
BrandMeister
  Parrot
  Local
  California
  USA
  Worldwide
```

Do not hard-code user-specific configuration as defaults.

## Channel generator

Given one DMR repeater:

```text
RX frequency
TX frequency
Color Code
```

generate multiple channels mapped to chosen talkgroups.

## Duplicate/clone channel

Useful for:

- changing talkgroup only
- changing timeslot only
- making local vs statewide variants

## Bulk edit

Allow multi-select changes for:

- power
- scan membership
- color code
- slot
- APRS state
- admit criteria
- bandwidth
- timeout timer

---

# 16. Phase 13 — Advanced APRS UX

The DB-25D Pro's real analog APRS support is a major feature and deserves first-class UI.

## Analog APRS wizard

Suggested flow:

```text
APRS Mode
   ↓
Callsign / SSID
   ↓
144.390 MHz preset for North America
   ↓
GPS or fixed location
   ↓
Path
   ↓
Beacon interval
   ↓
Power
   ↓
Symbol
   ↓
Assign APRS to analog channels
```

Provide presets but allow manual editing.

Possible regional preset system:

```text
North America: 144.390 MHz
Other regions: user selectable
```

Never silently assume transmit settings when importing an unknown codeplug.

## APRS status helper

Explain clearly:

```text
Analog APRS
uses 2m AFSK RF

DMR APRS
uses DMR network transport
```

That distinction should be obvious in the UI.

---

# 17. Phase 14 — Radio Selection and Auto-Detection

Long-term startup experience:

```text
Connect Radio
     ↓
Web Serial device picker
     ↓
probe supported protocols
     ↓
identify driver
```

If reliable automatic probing risks disturbing a radio, allow manual selection:

```text
Radio Type
  Auto Detect
  Baofeng DM-32UV
  Radioddity DB-25D Pro
```

Never send destructive/prolonged probes during automatic detection.

---

# 18. Phase 15 — Documentation

YWD-Plug should ship useful documentation in the repository from this point forward.

Suggested tree:

```text
docs/
  README.md
  getting-started.md

  radios/
    dm32uv.md
    db25d-pro.md

  programming/
    channels.md
    contacts.md
    zones.md
    scan-lists.md
    aprs.md

  troubleshooting/
    web-serial.md
    programming-cables.md
    failed-read.md
    failed-write.md

  development/
    architecture.md
    radio-driver-api.md
    codeplug-model.md
    db25d-protocol.md

  licensing/
    upstream-attribution.md
```

The DB25-D Pro documentation should explicitly cover:

- known supported firmware families
- known unsupported firmware families
- programming cable requirements
- official CPS backup recommendation
- how YWD-Plug backup files work
- analog vs digital APRS
- recovery steps after a failed programming session

---

# 19. Testing Strategy

This project will touch real radio flash. Tests matter.

## Unit tests

Test:

- encoding/decoding individual fields
- frequency conversion
- BCD handling
- string handling
- contact references
- zone references
- scan-list references
- APRS fields
- DMR fields
- checksums where applicable

## Fixture tests

Keep sanitized binary fixtures if redistribution is legally acceptable.

```text
test/fixtures/db25d/
  firmware-family-a.bin
  firmware-family-b.bin
```

If redistribution of raw vendor-generated codeplug images is questionable, keep local test fixtures excluded from Git and document how to generate them.

## Round-trip tests

Critical invariant:

```text
decode(original)
   ↓
encode(no changes)
   ↓
equivalent binary
```

For unknown/preserved regions, expect exact preservation.

## Hardware tests

Maintain a short hardware regression checklist:

### DM-32UV

- connect
- identify
- read
- export
- import
- safe write
- read back

### DB-25D Pro

- connect
- identify
- read
- decode
- backup
- edit
- validate
- write
- read back

---

# 20. Suggested Repository Structure

A possible evolution of the source tree:

```text
ywd-plug/
├── index.html
├── css/
│   ├── tokens.css
│   ├── base.css
│   ├── components.css
│   └── responsive.css
│
├── js/
│   ├── app.js
│   ├── router.js
│   ├── state.js
│   │
│   ├── transport/
│   │   ├── web-serial.js
│   │   └── serial-log.js
│   │
│   ├── radios/
│   │   ├── registry.js
│   │   ├── driver-base.js
│   │   │
│   │   ├── dm32uv/
│   │   │   ├── driver.js
│   │   │   ├── codec.js
│   │   │   └── capabilities.js
│   │   │
│   │   └── db25d/
│   │       ├── README.md
│   │       ├── driver.js
│   │       ├── protocol.js
│   │       ├── codec.js
│   │       ├── capabilities.js
│   │       └── layouts/
│   │
│   ├── model/
│   │   ├── radio-model.js
│   │   └── validation.js
│   │
│   ├── ui/
│   │   ├── navigation.js
│   │   ├── channels.js
│   │   ├── contacts.js
│   │   ├── zones.js
│   │   ├── scanlists.js
│   │   ├── aprs.js
│   │   ├── diagnostics.js
│   │   └── settings.js
│   │
│   └── files/
│       ├── ywdplug.js
│       ├── binary.js
│       └── csv.js
│
├── docs/
├── test/
├── LICENSE
├── NOTICE.md
└── README.md
```

This is a target architecture, not a requirement to rewrite everything immediately.

Refactor toward it incrementally.

---

# 21. Recommended Development Sequence

The practical order should be:

```text
A. Freeze and verify current YWD-Plug baseline
        ↓
B. Modernize theme/layout to match YWD-Hotspot
        ↓
C. Regression-test DM-32UV
        ↓
D. Extract current radio code behind driver interface
        ↓
E. Regression-test DM-32UV again
        ↓
F. Add GPL attribution/license structure
        ↓
G. Add DB-25D Pro probe
        ↓
H. Add DB-25D Pro raw read/backup
        ↓
I. Port/adapt codeplug decoder
        ↓
J. Verify decoder against CPS + CPEditor
        ↓
K. Build DB-25D read-only editors
        ↓
L. Add encode + offline binary export
        ↓
M. Validate controlled binary diffs
        ↓
N. Enable guarded radio write
        ↓
O. Add multi-radio `.ywdplug` schema
        ↓
P. Add advanced import/generator/APRS tooling
```

Do not collapse G through N into one giant commit.

---

# 22. Initial Milestones

## Milestone 1 — YWD-Plug UI Refresh

Success means:

- visually aligned with YWD-Hotspot
- responsive mobile/desktop UI
- unified controls
- existing DM-32UV functions unchanged

## Milestone 2 — Multi-Radio Architecture

Success means:

- DM-32UV runs through a driver interface
- UI is capability-aware
- Web Serial transport is radio-agnostic

## Milestone 3 — DB-25D Pro Detection

Success means:

- radio connects
- valid identification response displayed
- firmware/hardware info captured
- diagnostics export works
- no writes exist

## Milestone 4 — DB-25D Pro Backup

Success means:

- entire codeplug can be read
- binary backup saved
- repeat reads are stable
- no writes exist

## Milestone 5 — DB-25D Pro Decode

Success means:

- channels/zones/contacts/RX groups/scan lists/APRS decode correctly
- values verified against official CPS/CPEditor

## Milestone 6 — DB-25D Pro Editing

Success means:

- browser editor modifies an in-memory codeplug
- validation works
- binary can be generated offline
- no radio write until binary comparisons are proven

## Milestone 7 — DB-25D Pro Write

Success means:

- guarded write works
- automatic pre-write backup exists
- read-back confirms changes
- unknown firmware remains read-only

---

# 23. First Work Session in the YWD-Plug Project Space

When development resumes in the actual YWD-Plug repository, start here:

## Step 1

Inspect the repository without changing anything.

Capture:

```bash
git status --short --branch
git log -1 --oneline
find . -maxdepth 3 -type f | sort
```

## Step 2

Run the current application and regression-test the existing DM-32UV workflow.

## Step 3

Inspect the current UI/CSS structure and compare it with the current YWD-Hotspot UI sources.

Identify:

- reusable color values
- spacing
- cards
- navigation
- buttons
- toggles
- typography
- mobile breakpoints

## Step 4

Create a dedicated development branch if the project's current branching policy calls for one.

## Step 5

Make the first implementation goal **UI shell modernization only**.

Do not introduce DB-25D protocol work in the same initial change.

## Step 6

After the UI checkpoint passes, refactor Web Serial + DM-32UV into the new driver abstraction.

## Step 7

Only then begin the DB-25D Pro implementation.

---

# 24. Explicitly Deferred Work

Do not let these distract the first DB-25D implementation:

- firmware flashing
- firmware repository/mirroring
- radio firmware patching
- Bluetooth programming
- cloud accounts
- remote internet programming
- mobile native app wrappers
- unsupported radio families
- automatic repeater scraping without a clearly permitted data source
- deeply generalized "universal CPS" abstractions before two radios are working

Get **DM-32UV + DB-25D Pro** excellent first.

---

# 25. Project Philosophy

YWD-Plug should remain:

- browser-native
- local-first
- understandable
- inspectable
- easy to back up
- difficult to accidentally misuse
- friendly to people who hate traditional CPS software
- useful without requiring an account or cloud service

The long-term advantage is not merely "program radios from Chrome."

The advantage is:

> **A modern radio configuration model that understands what the user is trying to accomplish instead of exposing hundreds of cryptically named vendor fields.**

That means YWD-Plug can eventually provide higher-level workflows such as:

- "Add this DMR repeater"
- "Create channels for these talkgroups"
- "Enable analog APRS while mobile"
- "Build a scan list from this zone"
- "Clone this channel with a different talkgroup"
- "Show me broken references before I write the radio"

That is where YWD-Plug can become materially better than the vendor CPS rather than simply recreating it in a browser.

---

# 26. Immediate Checkpoint

The next development session should begin with exactly these priorities:

1. **Verify and freeze the existing working YWD-Plug baseline.**
2. **Revise the UI to match YWD-Hotspot while preserving all behavior.**
3. **Move the current DM-32UV implementation behind a radio-driver API.**
4. **Add GPL licensing/attribution structure.**
5. **Add DB-25D Pro probe/diagnostic support.**
6. **Do not write to the DB-25D Pro until read/decode/round-trip validation is complete.**

That sequence gives us the safest path from the current single-radio YWD-Plug into a polished multi-radio browser CPS without sacrificing the functionality that already works.
