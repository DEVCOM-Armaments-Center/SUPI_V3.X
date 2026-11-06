# How to contribute to sUPI

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

## Purpose of the reference design

The small Universal Payload Interface (sUPI) is a reference design. It is a common baseline for the
mechanical and electrical interface between a payload and a small uncrewed platform. A payload that
agrees with this baseline can move between all platforms that agree with it.

The baseline will change when platforms and payloads change. The users of the baseline tell us
which changes are necessary. This repository holds the golden copy of the baseline:

- The KiCad projects for the platform card and the payload card, with their fabrication and
  assembly files
- The released mechanical drawings and STEP models
- The platform MCU firmware source
- The integration briefs and the manufacturing notes

Before the maintainers accept a change, they examine its compatibility with v3.X hardware that is
in use. If a change makes a new half that cannot mate with an old half, the change must
have a strong reason. It also needs a new interface version.

## How to send feedback

Send all feedback through the GitHub
[Issues](https://github.com/DEVCOM-Armaments-Center/SUPI_V3.X/issues) and
[Pull Requests](https://github.com/DEVCOM-Armaments-Center/SUPI_V3.X/pulls) of this repository.

Open an issue for these items:

- A problem in the design
- Two documents that do not agree
- A part that you cannot make
- A question about the design or about how to install it
- A design change that you want to discuss before you start the work

Open a pull request for a correction or a change that you completed. Link the pull request to its
issue.

If you cannot use GitHub, send your feedback to your government point of contact.

## How to open an issue

1. Look at the open issues and the [`Known Issues/`](Documentation/Known%20Issues) folder.
2. If your problem is already there, add your information to that issue.
3. Write only one problem in each issue. Two problems on the same drawing are two issues.
4. Give the exact identification of the item:
   - The part number, assembly number or board number, and its revision
   - The file name
   - The sheet, the note or the reference designator

   Examples: `19200_13122518_v3-3.stp, hole C` and `100064667-001 Rev 1, U13`.
5. Use the same sections as the known issues. Then the maintainers can put an approved issue into
   `Documentation/Known Issues/` without changes:

   ```markdown
   <!-- Title: <part or document>: <the problem in one line> -->

   What is wrong. Give the evidence (measurements, text from the file, screenshots).

   ### Why this is a problem
   What goes wrong for the person who uses this data to make or install the item.

   ### Cause
   If you know the cause, write it here.

   ### Workaround
   What to do before the fix, if a workaround is possible.

   ### Fix
   - [ ] One change on each line.

   ### Check
   - [ ] How to make sure that the fix is correct.
   ```

6. Attach screenshots, marked-up drawings and measurements. These let other persons examine the
   problem.
7. Give measurements with sufficient decimal places to show the difference.

When the maintainers approve an issue, they add a file for it in `Documentation/Known Issues/`. They
put its images in `Documentation/Known Issues/assets/` and add an entry in
[`Known Issues/README.md`](Documentation/Known%20Issues/README.md). The title line of the file is
the same as the title of the GitHub issue. When the maintainers release the fix, they remove the
file. They move the entry to the list of closed issues.

## How to open a pull request

1. Open or find the related issue. You do not need an issue for an error in a word or for a
   broken link.
2. Write the issue number in the pull request description (`Fixes #12`).
3. Make a branch from `main`.
4. Put only one change in each pull request.
5. Write a description of the change.
6. Tell us if a v3.3 half made before the change mates with a half made after the change. Also
   tell us if the two halves operate correctly together.
7. Give the checks that you did and their results.

### What to include for each area

#### Circuit cards

1. Change the KiCad projects in `sUPI Electrical/`.
2. Change the KiCad project files (`.kicad_sch`, `.kicad_pcb`, `.kicad_pro`). The maintainers do
   not accept design files from other EDA tools.
3. If you made the change in other software, do not send a pull request. Open an issue. Attach a
   marked-up schematic or layout that shows the change.
4. In the same pull request, make again all the output files that the change affects. These are
   the schematic PDF, the fabrication drawing, the `_cam/` and `_stn/` data, the BOM and the
   pick-and-place file. If an output file does not agree with the source, it is a defect.
5. Do the ERC and the DRC. Give the results.
6. Do not change the part numbers. The maintainers give the revisions at release.

#### Mechanical parts

1. Change the STEP model in `sUPI Mechanicals/`.
2. In the same pull request, change the related drawing PDF.
3. Write a description of each change to the geometry. Include changes that are too small to see.
   The drawings make all model geometry BASIC, thus each change to the model is a change to the
   part.
4. Use metric values, or inch values with a minimum of 6 decimal places. Do not use the rounded
   dimensions from the drawing.
5. Add the change to `Documentation/CHANGELOG.md`.

#### Firmware

1. Change the source in `sUPI Firmware/Core/`.
2. If you change the STM32CubeMX configuration, commit the `.ioc` file and the new generated code
   together.
3. Build the firmware with the Keil MDK-ARM project.
4. Replace `build/supi_firmware.hex` with the new file.
5. Tell us the hardware that you used for the test and what you tested.

#### Documentation

1. Make sure that the briefs, the electrical and mechanical references, `README.md` and the
   manufacturing notes agree with the KiCad design and the released drawings.
2. If a document does not agree with the design data, the design data is correct. Change the
   document.
3. If the design data is wrong, open an issue.

## Data that you must not send

This repository is public under Distribution Statement A. Do not send these types of data in a
file, a comment or an attachment:

- Controlled or export-controlled information: CUI, ITAR or EAR technical data, or a document that
  has such a marking
- Proprietary data or third-party data that you do not have the right to release
- Personal information: names, telephone numbers, email addresses, or user names in file paths

Examine the hidden parts of the files and the visible content. These locations can contain this
type of data:

- PDF metadata and spreadsheet properties
- KiCad title blocks and text variables
- Absolute paths in netlists and 3D-model references
- Debug builds, for example `.axf` files

The maintainers close a submission that contains this type of data. They also remove the data.

## License

The maintainers accept contributions under the repository license in [`LICENSE.md`](LICENSE.md).
The vendor code in `sUPI Firmware/Drivers/` has its own licenses. Do not change the notices in
that code.
