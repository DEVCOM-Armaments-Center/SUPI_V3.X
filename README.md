# Small Universal Payload Interface (sUPI)

**Technical Data Package, Version 3.3**

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

sUPI connects one payload to one uncrewed platform. The platform is usually a small UAS. The sUPI
has two halves that latch together. Nine spring-pin contacts connect the halves. These contacts
carry battery power, USB, two 3.3 V PWM lines and a payload-detect line.

sUPI was developed for U.S. Army Special Operations Command. DEVCOM Armaments Center curates the
technical data package. CLIK Lite in Appendix A of the Picatinny Common Lethality Integration Kit
(CLIK) Design Standard uses the sUPI design as its model. CLIK Lite is for platforms that have limits on
size, weight and power, for example Short Range Reconnaissance and Purpose Built Attritable System
aircraft. The full CLIK rail and network interface is too large for these platforms.

| Half | Attaches to | Function |
| --- | --- | --- |
| Platform side | The vehicle | Controls and monitors the payload power, switches the USB, and holds the interface MCU |
| Payload side | The payload | Changes the platform battery voltage to the payload voltages, and holds the payload ID device |

## Repository contents

| Folder | Contents |
| --- | --- |
| [`Documentation/`](Documentation) | Integration briefs, electrical and mechanical references, conventions, change log, manufacturing notes |
| [`Documentation/Known Issues/`](Documentation/Known%20Issues) | Open errata, one file for each issue |
| [`sUPI Electrical/`](sUPI%20Electrical) | KiCad projects for the platform card and the payload card, with schematics, fabrication drawings, Gerber and drill data, stencil data, BOMs and pick-and-place files |
| [`sUPI Mechanicals/`](sUPI%20Mechanicals) | Released drawings and STEP models for assembly `13122513` and its five parts |
| [`sUPI Firmware/`](sUPI%20Firmware) | Platform MCU firmware source, Keil and CubeMX projects, and a built `.hex` file |

The KiCad projects and the released drawings are the golden source. If a document does not agree
with them, use the design data.

## Where to start

| If you | Read these documents in this order |
| --- | --- |
| Install the platform side | [Platform brief](Documentation/sUPI_Platform_Brief.md), assembly drawing, platform schematic |
| Install the payload side | [Payload brief](Documentation/sUPI_Payload_Brief.md), assembly drawing, payload schematic |
| Make the boards | [Known Issues](Documentation/Known%20Issues/README.md), `MFR_*.md` notes, [Electrical Reference](Documentation/Electrical_Reference.md), fabrication drawing, BOM and pick-and-place file |
| Machine the parts | [Known Issues](Documentation/Known%20Issues/README.md), [Mechanical Reference](Documentation/Mechanical_Reference.md), detail drawing and its STEP model |
| Work on the firmware | [Firmware README](sUPI%20Firmware/README.md), MCU sheet of the platform schematic, `Core/Src/supi.c` |
| Make a payload for sUPI | CLIK Design Standard, Appendix A (get it from the design activity), payload brief |

## Before you make boards

Five [known issues](Documentation/Known%20Issues/README.md) have an effect on the order and the
assembly of the boards. Three manufacturing notes change the BOMs:

- [`MFR_DNP.md`](Documentation/MFR_DNP.md): Do not install platform `U3`, `R1` to `R4`, `C2` and
  `JP1`. The undervoltage cutoff is not complete. It can stop the power to the payload.
- [`MFR_Rework.md`](Documentation/MFR_Rework.md): Remove platform `U11` and `U12`. Then connect
  pin 2 to pin 4 on each footprint. If you only remove the parts, the net stays open.
- [`MFR_Capacitors.md`](Documentation/MFR_Capacitors.md): Use the non-Murata alternative parts. The
  released BOMs still show the Murata part numbers.

The platform BOM and pick-and-place file still show all the parts that the first two notes remove.
Use the two notes with the BOM. Where they do not agree with the BOM, obey the notes.

## Version 3.3

On the mechanical side, v3.3 makes the parts easier to machine and makes the fit better. The
machining plan does not need special tools. The parts that mate have less play between them. On the
electrical side, v3.3 contains the KiCad reference design for the two cards and the platform
firmware source. Each v3.3 part has its version number engraved on it. Examine this number before
you mate halves from different versions. Refer to the [change log](Documentation/CHANGELOG.md).

## Contributions and license

sUPI is a reference design. It will change over time. To tell us about a problem or to send a
change, use GitHub issues and pull requests. Refer to [`CONTRIBUTING.md`](CONTRIBUTING.md). The
CERN Open Hardware Licence v2, Weakly Reciprocal, applies to the design ([`LICENSE.md`](LICENSE.md)).
