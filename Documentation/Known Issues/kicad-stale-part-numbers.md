<!-- Title: KiCad projects: title-block comments and helper files carry pre-release part numbers -->

The KiCad title-block comments on both cards give part numbers that are not the released numbers:

| Card | File | Title-block comment | Released number |
|---|---|---|---|
| Platform | `100064667.kicad_sch`, `100064667.kicad_pcb` | PWB PART `100043655-001` | PWB `100069026-001` |
| Platform | same | PWA PART `100046136-001` | PWA `100064667-001` |
| Payload | `100068622-001_Rev1.kicad_sch`, `.kicad_pcb` | PWB PART `100043659-001` | PWB `100068610-001` |
| Payload | same | PWA PART `100045703-001` | PWA `100068622-001` |

The payload project folder also has three helper files for the old PWB number: `100043659_fabnotes.json`, `100043659_numbers.json` and `100043659_stackup.json`. The released drawings use `100068622-001_Rev1_stackup.json` and the project text variables, not these files. The payload `.kicad_pro` STEP export name is also old: `100045703-001_Prelim.step`.

The platform sub-sheets keep symbol-instance data for the old project name: 81 instances use the old project name `100043655`. KiCad still exports the correct reference designators, so the design is not affected.

The released PDFs are not affected. Their title blocks use the `PWA_NUMBER`, `PWB_NUMBER` and `SCH_NUMBER` text variables, which have the correct numbers.

### Why this is a problem

A person who reads the KiCad title block, or uses the old helper files, can use the wrong part number. A plot from a drawing sheet that uses the comment fields shows the old numbers.

### Fix

- [ ] Change the title-block comments in both schematics and both boards to the released numbers.
- [ ] Remove the `100043659_*.json` files, or replace them with files for `100068610`.
- [ ] Change the payload STEP export name to `100068622-001_Rev1.step`.
- [ ] Open the platform project and save each sheet, so that KiCad writes the instances under project `100064667`. Then remove the `100043655` instances.

### Check

- [ ] The title blocks of both schematics and both boards give the released numbers.
- [ ] No `.kicad_sch`, `.kicad_pcb`, `.kicad_pro` or `.json` file contains `100043655`, `100046136`, `100043659` or `100045703`.
- [ ] A netlist export of each schematic gives the same reference designators as before.
