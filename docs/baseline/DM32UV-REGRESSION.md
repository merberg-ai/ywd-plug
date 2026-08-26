# DM-32UV Regression Checklist

Run this checklist at every milestone that could affect the existing radio path.

## Browser and connection

- [ ] Application loads without console-fatal errors.
- [ ] Browser reports Web Serial support.
- [ ] Radio picker opens from a user action.
- [ ] Programming cable can be selected.
- [ ] Connect/disconnect completes cleanly.
- [ ] Radio model/firmware information is displayed as expected.

## Read

- [ ] Full radio read completes.
- [ ] Progress/status feedback remains responsive.
- [ ] Channels populate correctly.
- [ ] Zones populate correctly.
- [ ] Scan lists populate correctly.
- [ ] Contacts / DMR data populate correctly.
- [ ] Radio settings populate correctly.
- [ ] Diagnostics/logging remain available.

## Files

- [ ] Export produces a `.ywdplug` file.
- [ ] The exported `.ywdplug` can be imported again.
- [ ] A known-good `.neonplug` compatibility file can still be imported.
- [ ] Importing/exporting does not unexpectedly alter unchanged radio data.

## Write safety

Only perform this section with a safe test radio and a known-good backup.

- [ ] Fresh radio read/backup exists before write.
- [ ] Intended edit is small and easily verifiable.
- [ ] Write completes without protocol errors.
- [ ] Radio remains operational after write.
- [ ] Read-back completes.
- [ ] Read-back contains the intended change.
- [ ] Unrelated values remain unchanged.

## Gate

If any previously-working item fails, stop the next milestone and treat it as a regression until explained and fixed. Do not hide a radio regression behind UI cleanup or DB-25D work.
