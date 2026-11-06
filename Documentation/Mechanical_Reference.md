# sUPI Mechanical Reference

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

The drawings in `sUPI Mechanicals/` control the part definition. The dimensions are in inches.
Interpret them in accordance with ANSI/ASME Y14.5M-2009.

Each detail drawing gives the name of a design model. This model is necessary for the full product
definition. Features without dimensions on the drawing get their dimensions from this model.

- Do not machine a part from the PDF drawing only.
- Read the [Known Issues](Known%20Issues/README.md) before you machine a part.

## Installation

Use M3 × 0.5 mm, 8 mm long, black-oxide stainless steel Phillips flat-head screws to attach the two
halves.

| Half | Screws | Drawing |
| --- | --- | --- |
| Platform side | 3X M3 | `19200_13122518` |
| Payload side | 4X M3 | `19200_13122514` |

You can use one of these two methods to install each half:

1. Direct installation: Drill and tap the sUPI hole pattern into the host structure.
2. Adapter: Make an adapter. Put the sUPI hole pattern on one face of the adapter. Make the other
   face agree with an interface that is on the host. Use this method if you cannot change the host
   structure.

> **WARNING:** Align the platform CG and the payload CG when you install the two halves. If these
> CG points are not aligned, there can be critical failures.

Each half has a CG reference point:

- Platform side: Figure 2 of the [platform brief](sUPI_Platform_Brief.md) shows the `VEHICLE CG`
  callout. This point is on the centerline, 1.099 in from the top edge.
- Payload side: Figure 2 of the [payload brief](sUPI_Payload_Brief.md) shows the `VEHICLE CG`
  callout.

Install each half so that the center of gravity is as near to its reference point as possible.

## Part tree

Assembly `13122513` has one of each of these parts:

| Part No. | Nomenclature | Material | Process / Finish |
| --- | --- | --- | --- |
| `13122514` | V-Mount Base | 6061-T6 per ASTM B209, or polymer powder bed fusion | Bead blast; hard anodize per MIL-A-8625F Type III Class 2, black, sealed |
| `13122515` | Short Button | 6061-T6 per ASTM B209 | Bead blast; hard anodize per MIL-A-8625F Type III Class 2, black, sealed |
| `13122516` | Small Button Cover | PA12 or PA6 | Polymer powder bed fusion (MJF, SLS, or SAF); depowder, media blast, optional black dye |
| `13122517` | Connector Cover | 6061-T6 per ASTM B209, or polymer powder bed fusion | Bead blast; hard anodize per MIL-A-8625F Type III Class 2, black, sealed |
| `13122518` | V-Mount | 6061-T6 per ASTM B209 | Mask/plug threaded holes, then bead blast and hard anodize per MIL-A-8625F Type III Class 2, black, sealed |

Threaded holes and clearance holes:

- `13122514`: 4× M3 clearance, 90° countersunk; 2× M2 × 0.4 tapped
- `13122517`: 2× M2 × 0.4 clearance
- `13122518`: 3× M3 clearance, 90° countersunk; 3× M1.6 × 0.35 tapped, through

Each v3.3 part has its version number engraved on it. Examine this number before you mate halves
from different versions. Parts from earlier versions have a looser fit. Earlier `13122515` buttons
are taller, and they can bend. Refer to [`CHANGELOG.md`](CHANGELOG.md) for the design changes in
each version.

## How to change the parts

The STEP models are the mechanical source.

1. Import the STEP model into a CAD tool that can read STEP.
2. Change the model.
3. Release the new STEP model with its drawing.

The drawings make all model geometry BASIC. Thus, each change to the model is a change to the part.

## CLIK Design Standard

The interface envelope comes from Appendix A of the Picatinny CLIK Design Standard. The design
activity holds the standard. This repository does not contain a copy. Get the current revision
from the design activity.
