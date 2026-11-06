<!-- Title: Payload 100068622: the BOM lists R14 (DNP) and does not list R1 -->

The payload BOM (`100068622-001_Rev1_bom.xlsx`) has this line for the 100 kΩ resistors:

| Ref | Qty | Value | MPN |
|---|---|---|---|
| `R2,R13,R14` | 3 | 100K 1% 0.1W | `ERJ-2RKF1003X` |

The KiCad design does not agree:

- The schematic and the board mark `R14` (Rail Discharge sheet) as DNP. The pick-and-place does not include it.
- The design installs `R1` (Power Supplies sheet). It has the same value and MPN. The pick-and-place includes it, but the BOM does not.

The correct line is `R1,R2,R13`. The quantity of 3 is correct.

`C20` is also marked DNP. The BOM and the pick-and-place correctly leave it out.

### Why this is a problem

An assembler who loads the BOM by reference designator populates `R14` and leaves `R1` empty.

### Cause

The BOM is probably older than the DNP flags on `R1` and `R14`.

### Workaround

Build from the pick-and-place reference designators. Populate `R1`. Do not populate `R14` or `C20`.

### Fix

- [ ] Generate the payload BOM again from `100068622-001_Rev1.kicad_sch`.

### Check

- [ ] The 100 kΩ line reads `R1,R2,R13`.
- [ ] Every reference designator in the BOM is in the combined pick-and-place sheet, and the other way around (except fiducials, pads and pogo pins).
