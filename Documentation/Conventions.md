# sUPI Conventions

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

## Part numbers and file names

- Mechanical parts have 8-digit part numbers. The file name has CAGE code `19200` at the start and
  the TDP version (`_v3-3`) at the end, for example `19200_13122514_v3-3.pdf`. The assembly STEP
  file name also has `_asm`: `19200_13122513_asm_v3-3.stp`.
- Each circuit card has three 9-digit numbers: the assembly (PWA), the bare board (PWB) and the
  schematic.
- PWA and PWB file names have a `-001` configuration and a revision, for example
  `100069026-001_Rev1`. Schematic file names have only the revision, for example
  `100046138_Rev2.pdf`.
- The fabrication files and the stencil files use the PWB number. The BOM and the pick-and-place
  file use the PWA number.

## Units and standards

- The units on mechanical drawings are inches, unless the drawing gives a different unit.
  Tolerances are in accordance with ASME Y14.5M-2009. All model geometry is BASIC.
- Interpret board drawings in accordance with ASME Y14.5M. Make the boards in accordance with
  IPC-6012 Class 2.
- Each type of board data file uses its own unit:
  - Gerber files: millimetres
  - Drill files and pick-and-place coordinates: inches
  - Stackup, impedance and net-tie tables: mils
- Mechanical parts: packaging in accordance with MIL-STD-2073-1 Method 10, and marking in
  accordance with MIL-STD-130.
- Bare boards: after a board passes the electrical test, mark it `ET` in accordance with
  MIL-STD-130.

## File types

- Mechanical: `.pdf` released drawing, `.stp` solid model. The STEP model is the mechanical source
  that you can change.
- Electrical source: KiCad project files (`.kicad_pro`, `.kicad_sch`, `.kicad_pcb`), saved in
  KiCad 8 format.
- Electrical outputs: `.pdf` schematics, fabrication drawings, layer plots and stencil drawings;
  Gerber X2; Excellon drill; IPC-D-356 netlist (`.d356`); `.xlsx` BOM and pick-and-place file.
- KiCad title blocks get their data from project text variables in each `.kicad_pro` file. Examples
  are `PWA_NUMBER`, `PWB_NUMBER`, `SCH_NUMBER`, revisions and titles. To change a title block,
  change the variables. Do not change the drawing sheet.
- KiCad finds vendor 3D models through the path `${SUPI_3DMODEL_DIR}`.
- Firmware: C source, a Keil MDK-ARM project and an STM32CubeMX `.ioc` file. The built image is in
  Intel HEX format.

## Documentation

- All documents are in `Documentation/`, except `README.md`, `CONTRIBUTING.md` and `LICENSE.md`.
  Their figures are in `Documentation/assets/`.
- Each known issue has its own file in `Documentation/Known Issues/`. The file name starts with the
  part or document that has the issue, for example `13122518-hole-spacing-issue.md`. The first line
  of the file is a `<!-- Title: … -->` line. The figures for the issues are in
  `Documentation/Known Issues/assets/`.
