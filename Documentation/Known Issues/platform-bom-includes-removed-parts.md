<!-- Title: Platform 100064667: the BOM and pick-and-place still include parts that the MFR notes remove -->

Two manufacturing notes change the parts that you install on the platform card:

- [`MFR_DNP.md`](../MFR_DNP.md): do not populate `U3`, `R1`, `R2`, `R3`, `R4`, `C2`, `JP1` (undervoltage cutoff).
- [`MFR_Rework.md`](../MFR_Rework.md): remove `U11` and `U12`, then bridge pin 2 to pin 4 on each footprint.

The released platform BOM (`100064667-001_Rev1_bom.xlsx`) and pick-and-place (`100064667-001_Rev1_pos.xlsx`) list all 9 of these parts as populated. The KiCad schematic does not mark them as DNP.

### Why this is a problem

An assembler who builds from the BOM and pick-and-place builds a card that the notes say can block payload power. `U11` and `U12` then need rework after assembly.

### Workaround

Give the two MFR notes to the assembler with the BOM. Tell the assembler not to install `U3`, `R1` to `R4`, `C2`, `JP1`, `U11` and `U12`. Tell the assembler to install the pin 2 to pin 4 bridges at `U11` and `U12`.

### Fix

- [ ] Decide if the MFR notes become the design baseline.
- [ ] If yes: mark the 9 parts DNP in `100064667.kicad_sch` and add the `U11` and `U12` bridges to the design. Generate the BOM and pick-and-place again.

### Check

- [ ] The BOM and the pick-and-place do not contain the 9 parts.
- [ ] The assembled card agrees with the effect that `MFR_Rework.md` gives.
