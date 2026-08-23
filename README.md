# sUPI — Small Universal Payload Interface

**Technical Data Package, Version 3.3**

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

---

## Overview

sUPI is an interchangeable mechanical and electrical interface for attaching payloads to uncrewed
systems, primarily small UAS. It is a one-to-one peripheral interface: one platform, one payload, no
network.

sUPI was developed under U.S. Army Special Operations Command and has since been transitioned to
DEVCOM Armaments Center. The **CLIK Lite** adaptation in Appendix A of the Picatinny Common
Lethality Integration Kit (CLIK) Design Standard is modeled on sUPI. CLIK Lite exists for platforms
constrained in size, weight, and power — Short Range Reconnaissance and Purpose Built Attritable
System aircraft — where the full CLIK rail-and-network interface does not fit.

The standard is not redistributed here; request the current revision from the design activity.

sUPI is a **reference design**, intended to evolve. See `MFR_Reference_Design.md` for how to submit
recommendations.

### The Two Halves

| Half | Mounts to | Role |
| --- | --- | --- |
| **Platform side** | The vehicle | Gates and monitors payload power, switches USB, hosts the interface MCU |
| **Payload side** | The payload | Regulates platform battery to payload rails, carries the payload ID device |

Start with [`sUPI_Platform_Brief.md`](sUPI_Platform_Brief.md) or
[`sUPI_Payload_Brief.md`](sUPI_Payload_Brief.md) depending on which side you are integrating.

### Circuit Cards

| Card | Project | Assembly | Layers | Notes |
| --- | --- | --- | --- | --- |
| Platform | `PB-F-0213` | `PB-F-0210` A.6 (`100064667`) | 8 | The only platform-side card |
| Payload — Fusion | `PB-F-0212` | `PB-F-0209` A.3 (`100068622-001`) | 4 | Full-capability payload card |
| Payload — Medium R2 | `PB-F-0230` | `PB-F-0230` A.6 | 2 | Compact option, adds USB-to-UART |

---

## Electrical Interface

### Platform → Platform-Side Card

| Signal | Range |
| --- | --- |
| `VBATT` / `VBATT_RET` | 13 – 25.2 VDC (4S–6S pack) |
| `GPIO` | Typically PWM, ±5 VDC |
| `USB` | 0 – 3.3 VDC |

### Payload-Side Card → Payload

| Signal | Range |
| --- | --- |
| `VBATT` / `VBATT_RET` | 13 – 25.2 VDC, passed through |
| `V1` | Regulated **3.3 or 5 VDC** — set by solder bridge |
| `V2` | Regulated **5, 7.4, or 12 VDC** — set by solder bridge |
| `GPIO` | Typically PWM, ±5 VDC |
| `USB` | 0 – 3.3 VDC |

> **V1 and V2 are hardware-selected, not negotiated.** The rails are set by solder-bridge placement
> at build time. Verify the bridges match the payload before first power-on.

Both cards use solder pads for field wiring: 18–22 AWG for power, 24–30 AWG for signals.

### Board Markings

- **Platform pads:** `VBATT`, `GND`, `GPIO 0`, `GPIO 1`, `+USB−`, `AUX`
- **Platform jumpers:** `DETECT PAYLOAD`, `SELECT`, `FORCE ON`, `LO BATT CUTOFF`
- **Payload pads:** `VBATT`, `GND`, `V1`, `V1 OK`, `V2`, `V2 OK`, `USB DP`, `USB DM`, `GPIO 0`, `GPIO 1`
- **Payload selectors:** `V1SEL` (`3V3` / `5V`), `V2SEL` (`5V` / `7V4` / `12V`)

`FORCE ON` overrides the detect gate — confirm its state before flight.

### Mating Connection

A **9-contact, 0.100"-pitch right-angle spring-pin interface.** The payload side carries the spring
pins (Mill-Max 829 series); the platform side carries the mating targets (Mill-Max 399 series).
Power crosses on separate, larger single-pin contacts.

---

## Platform Card — `PB-F-0210` A.6

8-layer FR4, 0.5 oz outer / 1 oz inner copper, 0.0586 in finished thickness, ENIG per IPC-4552,
green mask. Controlled impedance: 90 Ω ±10 % differential on layer 5, referenced to layers 4 and 6.
Fabrication and inspection per MIL-PRF-31032/1 Type 3.

| Function | Device |
| --- | --- |
| Interface MCU | STM32C011F6 (Arm Cortex-M0+) |
| Payload power switch | MPQ5069, 7 mΩ protection switch with current monitor |
| Current sense | ACS711, ±15 A bidirectional Hall-effect with overcurrent fault output |
| Ideal-diode / reverse-polarity control | NCV68261 driving an NTTFS2D1N04HL N-channel MOSFET |
| Undervoltage supervisor | LTC2965 |
| Supply supervisor | MAX821, 2.78 V |
| Detect / enable logic | SN74LVC1G08 AND gate, SN74AUP2G04 inverter |
| Logic 3.3 V rail | NCP730 LDO, 150 mA |
| USB routing | TC7USB40MU dual SPDT, MAX20336 DPST |
| Transient protection | SMF26A 26 V TVS on `VBATT`, SMF3.3 on signals, PESD5V0C2UM on USB |

---

## Payload Cards

Two payload-side cards are released. Both present the same interface to the platform and both carry
the DS2431 1-Wire EEPROM that the platform MCU reads to identify an attached payload, so a payload
built to this interface inherits its identity device from the card.

### Fusion Payload — `PB-F-0209` A.3

The full-capability card. 4-layer FR4, 1 oz copper, 0.0442 in finished thickness, ENIG, green mask.
Controlled impedance 100 Ω ±10 % differential. Fabrication per MIL-PRF-31032/1 Type 3.

| Function | Device |
| --- | --- |
| V1 / V2 regulation | MAX25254, dual 36 V input / 0.8–14 V output synchronous buck, XEL4030 0.47 µH inductors |
| Payload ID / detect | DS2431 1-Wire EEPROM, 1 kbit |
| Input protection | NCV68261 + NTTFS2D1N04HL N-channel MOSFET, DMP610DL P-channel blocking FET |
| Logic 3.3 V rail | NCP730 LDO, 150 mA |
| Supply supervisor | MAX821, 2.78 V |
| USB routing | TC7USB40MU dual SPDT, MAX20336 DPST |
| Transient protection | SMF26A 26 V TVS on `VBATT`, SMF3.3 on signals, PESD5V0C2UM on USB |

### Medium R2 Payload — `PB-F-0230` A.6

A compact alternative for payloads that do not need the dual selectable rails. 2-layer 370HR FR4,
1 oz copper, ENIG, green mask, vias unfilled, no board-manufacturer markings, assembly serial
number silkscreened. Assembly to IPC J-STD-001 Class 3 and IPC-A-610 Class 3.

**Tested against all sUPI v3.X releases.**

| Function | Device |
| --- | --- |
| Regulation | MCP16361, 48 V 2 MHz buck, 2.2 µH inductor |
| Payload power switch | NCP380, fixed current-limiting power distribution switch |
| Supply supervisor | TPS389030 |
| Payload ID / detect | DS2431 1-Wire EEPROM, 1 kbit |
| **USB-to-UART bridge** | **FT232RN** — converts the interface USB link to a serial port on the payload |
| Wire-to-board connectors | 2× JST SM04B-SRSS-TB, 4-position 1.25 mm |
| Status indication | 2× red 625 nm LEDs |
| Mating contacts | 9-contact right-angle spring-pin header |
| Transient protection | SMF26A 26 V TVS, P4FL3.3A on signals, SS3060HE Schottky |

The USB-to-UART bridge is the substantive difference: a payload using the Medium card gets a serial
interface without implementing USB itself.

**Variant.** One variant assembly is released, `PB-F-0230` A.4. It is identical to A.6 except that
`R6` and `R7` — two 0 Ω links — are not populated. Check which assembly you have before assuming a
link is present.

---

## Mechanical Interface

Both halves mount with **M3 × 0.5 mm thread, 8 mm long, black-oxide stainless steel Phillips
flat-head screws.**

| Half | Screws | Controlling drawing |
| --- | --- | --- |
| Platform side | 3X M3 | `19200_13122518` |
| Payload side | 4X M3 | `19200_13122514` |

Two accepted installation methods, either half:

1. **Direct mount** — drill and tap the sUPI hole pattern into the host structure.
2. **Adapter** — fabricate an adapter carrying the sUPI hole pattern on one face and mating to an
   existing interface on the other. Preferred where the host structure cannot be modified.

The payload-side pattern carries a CG reference callout. Position the interface so the center of
gravity lands as close to that point as the installation allows.

### Part Tree

Assembly **13122513**, one each of:

| Part No. | Nomenclature | Material | Process / Finish |
| --- | --- | --- | --- |
| `13122514` | V-Mount Base | 6061-T6 per ASTM B209, or polymer powder bed fusion | Bead blast; hard anodize per MIL-A-8625F Type III Class 2, black, sealed |
| `13122515` | Short Button | 6061-T6 per ASTM B209 | Bead blast; hard anodize per MIL-A-8625F Type III Class 2, black, sealed |
| `13122516` | Small Button Cover | PA12 or PA6 | Polymer powder bed fusion (MJF, SLS, or SAF); depowder, media blast, optional black dye |
| `13122517` | Connector Cover | 6061-T6 per ASTM B209, or polymer powder bed fusion | Bead blast; hard anodize per MIL-A-8625F Type III Class 2, black, sealed |
| `13122518` | V-Mount | 6061-T6 per ASTM B209 | Mask/plug threaded holes, then bead blast and hard anodize per MIL-A-8625F Type III Class 2, black, sealed |

Threaded and clearance features:

- `13122514` — 4× M3 clearance, 90° countersunk; 2× M2 × 0.4 tapped
- `13122517` — 2× M2 × 0.4 clearance
- `13122518` — 3× M3 clearance, 90° countersunk; 3× M1.6 × 0.35 tapped, through

Dimensions are in inches; interpret per ANSI/ASME Y14.5M-2009. **The drawings are the authority** —
each detail sheet names a design model required to complete the product definition, and
undimensioned features are taken from that model. Do not machine from the PDFs alone.

---

## Before You Fabricate Boards

| Note | Direction |
| --- | --- |
| [`MFR_DNP.md`](sUPI%20Electrical/MFR_DNP.md) | Do not populate platform `U3`, `R1`–`R4`, `C2`, `JP1` — the undervoltage cutoff is still in development and can block power to the payload |
| [`MFR_Rework.md`](sUPI%20Electrical/MFR_Rework.md) | Remove platform `U11` and `U12` and bridge pin 2 to pin 4 on each footprint. The bridge is the workaround; removal alone leaves the net open |
| [`MFR_Capacitors.md`](sUPI%20Electrical/MFR_Capacitors.md) | Superseded — its alternates are already in the released BOMs and no Murata parts remain. Build from the BOM. |

All three components named by the do-not-populate and rework notes are still present in the released
platform BOM, so both notes remain live direction.

With that direction applied, the platform card has **no undervoltage supervision and no supply
supervisor**, and its detect/enable gate is strapped to a permanent pass-through. If your
application depends on brownout protection or supervised power sequencing, treat that as an open
engineering item.

The Medium R2 assembly carries its own do-not-populate list on the assembly drawing. For the base
A.6 assembly that list is empty; for the A.4 variant it is `R6`, `R7`.

---

## Firmware

`supi_firmware.hex` is the platform MCU image: 20,440 bytes, load address `0x08000000`, targeting
the STM32C011 (Arm Cortex-M0+). Intel HEX format.

---

## Revision History

See [`CHANGELOG.md`](CHANGELOG.md). Known documentation defects are listed in
[`KNOWN_ISSUES.md`](KNOWN_ISSUES.md).

v3.3 is a manufacturability and fit release on the mechanical side — it removes special tooling from
the machining plan and takes slop out of the mating stack. On the electrical side it adds the
Medium R2 payload card and re-releases all three boards with updated component sourcing.

**v3.3 hardware is physically marked.** Check the engraved version number before mating
mixed-vintage halves: earlier parts have looser fit, and earlier `13122515` buttons have the taller
profile that was prone to bending.

---

## Package Contents

```
.
├── CHANGELOG.md                     Mechanical design change history, v3.1 → v3.3
├── KNOWN_ISSUES.md                  Errata — read before fabricating or machining
├── MFR_Reference_Design.md          Scope of the reference design; how to submit feedback
├── sUPI_Platform_Brief.md           Platform-side integration brief
├── sUPI_Payload_Brief.md            Payload-side integration brief
├── assets/                          Figures for the briefs and changelog
│
├── sUPI Mechanicals/
│   ├── 19200_13122513_v3-3.pdf      Assembly drawing
│   ├── 19200_1312251[4-8]_v3-3.pdf  Detail drawings
│   └── 19200_*_v3-3.stp             Solid models (STEP)
│
├── sUPI Electrical/
│   ├── MFR_DNP.md                   Do-not-populate direction
│   ├── MFR_Rework.md                Platform rework work instructions
│   ├── MFR_Capacitors.md            Capacitor sourcing history
│   ├── Platform/PB-F-0213 …/        Altium project: SRC (schematics, PCB), FAB, ASM
│   └── Payload/
│       ├── PB-F-0212 …/             Fusion payload: SRC, FAB, ASM
│       └── PB-F-0230 …/             Medium R2: SRC, FAB, ASM (base and variant)
│
└── sUPI Firmware/
    └── supi_firmware.hex            Platform MCU image
```

Each board's `FAB-` folder carries the fabrication drawing, GerberX2, IPC2581, ODB++, and NC Drill
data. Each `ASM-` folder carries the assembly drawing and the bill of materials with pick-and-place.

### Where to Start

| You are | Read, in order |
| --- | --- |
| **Platform integrator** | Platform brief → assembly drawing → platform schematics |
| **Payload integrator** | Payload brief → assembly drawing → pick a payload card → its schematics |
| **Fabricating boards** | `MFR_*.md` notes → fabrication drawing → Gerbers / ODB++ → BOM |
| **Machining parts** | Detail drawing + its matching STEP model |
| **Bringing up firmware** | Platform schematics, MCU section → `supi_firmware.hex` |
| **Writing a compliant payload** | Picatinny CLIK Design Standard, Appendix A (request from the design activity), then the payload brief |

---

## Conventions

- **Mechanical part numbers** are 8-digit design-activity numbers, prefixed in filenames with CAGE
  code `19200` and suffixed with the TDP version (`_v3-3`).
- **Circuit card part numbers** are `PB-F-####`. Project folders carry the project number; the
  fabrication and assembly folders inside carry their own numbers and an `A.n` revision.
- **Dimensions** are in inches unless otherwise specified; mechanical tolerances per
  ANSI/ASME Y14.5M-2009, board drawings per ASME Y14.5M-1994 and IPC-2221/2222.
- **Mechanical files:** `.pdf` released drawing, `.stp` neutral solid model.
- **Electrical files:** `.PDF` fabrication and assembly drawings, `.SchDoc` / `.PcbDoc` Altium
  source, GerberX2, `.cvg` IPC2581, ODB++ archives, NC Drill, `.xlsx` BOM and pick-and-place.
- **Packaging and marking** per MIL-STD-2073-1 Method 10 and MIL-STD-130.
