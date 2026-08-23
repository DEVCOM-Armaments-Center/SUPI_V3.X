# sUPI v3.3 — Known Issues

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

Errata for the v3.3 technical data package. Where a document contradicts itself, this file says
which source to trust. Report additions per `MFR_Reference_Design.md`.

---

## Read this before fabricating or machining

### Payload-side screw count

The payload brief specifies mounting via **3X** M3 screws, then directs the installer to drill and
tap **4X** holes in the same paragraph.

**Four is correct.** The hole-pattern figure shows four countersunk M3 clearance holes, and drawing
`19200_13122514` calls out 4X ⌀.134 THRU with a .248 × 90° countersink. `sUPI_Payload_Brief.md`
carries the corrected count. The platform side is genuinely 3X.

The same 3X/4X contradiction appears in Appendix A of the Picatinny CLIK Design Standard, which
inherited the text from this brief.

### Payload fabrication drawing — impedance note references layers that do not exist

`PB-F-0209` is a **4-layer** board. Note 13 on its fabrication drawing specifies controlled
impedance "on Layer 1, 8 with reference to Layer 2,7" — a callout carried from an 8-layer template.

**The 100 Ω ±10 % differential target is believed correct; the layer assignment is wrong.** Confirm
with the design activity before fabricating impedance-controlled boards.

### Mechanical drawings — stale design model in note 5

Note 5 on each detail drawing names a design model required to complete the product definition. Two
sheets name a superseded model:

| Drawing | Note 5 cites | Current model |
| --- | --- | --- |
| `19200_13122514` | `100051078` | **`100068609`** |
| `19200_13122517` | `100051078` | **`100068617`** |
| `19200_13122515` | `100064656` | correct |
| `19200_13122516` | `100051265` | correct |
| `19200_13122518` | `100064665` | correct |

Neither design model is included in this release — see
[Limits on modifying the design](#limits-on-modifying-the-design).

### Material and finish conflict on two parts

`19200_13122514` and `19200_13122517` both permit **6061-T6 aluminium or polymer powder bed fusion**
while specifying hard anodize per MIL-A-8625F, which is not applicable to polymer. `13122517`
additionally mandates additive manufacturing in note 2 while permitting aluminium in note 3.

Treat the finish requirement as conditional on choosing aluminium. Both sheets need a
material-conditional finish note.

### Bills of material are superseded by the manufacturing notes

`MFR_DNP.md` and `MFR_Rework.md` carry live direction that changes what gets populated on the
platform card. Read them before ordering assembly — see the README section *Before You Fabricate
Boards* for the effect on the delivered card. `MFR_Capacitors.md` is superseded; build from the BOM.

---

## Documentation defects

| Item | Detail |
| --- | --- |
| **Figures show V2 hardware** | The platform wiring figure shows a board silkscreened `SUPI V2` despite the actual board being V3, both hole-pattern figures are engraved `V3` rather than v3.3. |
| **Clipped figure** | The platform wiring figure is clipped in the source document — an internal caption reads `…und`, truncated from "Ground". |
| **BOM part description** | The platform BOM describes `U5` (`STM32C011F6U6TR`) as a "CORTEX-M4F CPU". The STM32C011 is **Arm Cortex-M0+**. Part number is correct; only the description is wrong. |
| **Stale project reference** | Two of the three Altium projects reference a `PCB2.PcbDoc` that is not part of the package. Opening either project reports a missing document; it is harmless and predates this release. |

---

## Limits on modifying the design

| | |
| --- | --- |
| **Mechanical parts can be manufactured, not revised.** | The release carries released drawings and STEP models. STEP is boundary-representation geometry, so revising a part requires the parametric source, which is held by the design activity along with the design models named in note 5. There is current effort to add the true design files to the repo. |
| **Circuit cards can be modified.** | The Altium projects are complete and editable: `.PrjPcb`, `.SchDoc`, `.PcbDoc`, `.SchLib`, `.BomDoc`, and the OutJob definitions that generate the fabrication and assembly outputs. A KiCad import reads these directly. |
| **The Picatinny CLIK Design Standard is held by the design activity.** | Request the current revision from them. The interface envelope summarised in the README is drawn from it. |
