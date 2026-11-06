# sUPI Electrical Reference

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

The electrical reference design has one KiCad project for each side of the connector. These
projects are the golden source. If a document does not agree with the KiCad design, use the KiCad
design.

Read the [Known Issues](Known%20Issues/README.md) and the `MFR_*.md` notes before you order or
assemble boards.

| Card | Assembly (PWA) | Bare board (PWB) | Schematic | Layers |
| --- | --- | --- | --- | --- |
| Platform | `100064667-001` Rev 1 | `100069026-001` Rev 1 | `100046138` Rev 2 | 8 |
| Payload | `100068622-001` Rev 1 | `100068610-001` Rev 1 | `100046137` Rev 2 | 4 |

## Files

The folder of each card in `sUPI Electrical/` contains these files:

| What | Platform | Payload |
| --- | --- | --- |
| KiCad project | `100064667-001_Rev1/` | `KiCAD_100068622-001_Rev1/` |
| Schematic | `100046138_Rev2.pdf` | `100046137_Rev2.pdf` |
| Fabrication drawing | `100069026-001_Rev1.pdf` | `100068610-001_Rev1.pdf` |
| Fabrication data and layer plots | `100069026-001_Rev1_cam/`, `_cam.pdf` | `100068610-001_Rev1_cam/`, `_cam.pdf` |
| Stencil data and pick-and-place | `100069026-001_Rev1_stn/`, `_stn.pdf` | `100068610-001_Rev1_stn/`, `_stn.pdf` |
| BOM and pick-and-place | `100064667-001_Rev1_bom.xlsx`, `_pos.xlsx` | `100068622-001_Rev1_bom.xlsx`, `_pos.xlsx` |

## Signals

### Platform to platform-side card

| Signal | Range |
| --- | --- |
| `VBATT` / `VBATT_RET` | 13 to 25.2 VDC (4S to 6S pack) |
| `GPIO` | 3.3 V PWM, 50 Hz |
| `USB` | 0 to 3.3 VDC |

> **CAUTION:** Ensure that 'DETECT' is enabled via solder blob before installing to a platform.

`DETECT` is the default configuration for the platform-side card. It is what enables the capability to "hot plug" payloads to a powered platform safely. `FORCE ON` overrides the detect gate and keeps the power pins hot no matter the connection state.

### Payload-side card to payload

| Signal | Range |
| --- | --- |
| `VBATT` / `VBATT_RET` | 13 to 25.2 VDC, passed through |
| `V1` | Regulated 3.3 or 5 VDC, set by solder bridge |
| `V2` | Regulated 5, 7.4, or 12 VDC, set by solder bridge |
| `GPIO` | 3.3 V PWM, 50 Hz |
| `USB` | 0 to 3.3 VDC |

> **CAUTION:** Make sure that the `V1SEL` and `V2SEL` solder bridges agree with the payload before
> you apply power for the first time.

The two cards have solder pads for the field wires. Use 18 to 22 AWG wire for power. Use 24 to 30
AWG wire for signals.

### Board markings

- Platform pads: `VBATT`, `GND`, `GPIO 0`, `GPIO 1`, `+USB−`, `AUX`
- Platform jumpers: `DETECT PAYLOAD`, `SELECT`, `FORCE ON`, `LO BATT CUTOFF`
- Payload pads: `VBATT`, `GND`, `V1`, `V1 OK`, `V2`, `V2 OK`, `USB DP`, `USB DM`, `GPIO 0`, `GPIO 1`
- Payload selectors: `V1SEL` (`3V3` / `5V`), `V2SEL` (`5V` / `7V4` / `12V`)

### Connection between the halves

The halves mate through a 9-contact, 0.100 in pitch, right-angle spring-pin interface.

- The payload side has the spring pins. All of them are Mill-Max 829 series.
  - A 5-pin header (`P3`) carries `DETECT`, `GPIO 0`, `GPIO 1` and the two USB lines.
  - Four single-pin contacts carry the power: `P1` and `P5` on `VBATT`, `P2` and `P4` on `GND`.
- The platform side has one 9-position Mill-Max `399-10-109-10-007000` target strip (`J10`). It
  mates with all nine spring pins.

## Board construction

The two cards have these specifications:

- Material: 370HR FR4
- Finish: ENIG in accordance with IPC-4552
- Soldermask: green LPI, halogen free
- Silkscreen: white
- Fabrication: in accordance with IPC-6012 Class 2
- Electrical test: 100 % in accordance with IPC-9252 Class 2, against the supplied IPC-D-356
  netlist

| | Platform PWB `100069026` | Payload PWB `100068610` |
| --- | --- | --- |
| Copper layers | 8 | 4 |
| Copper weight | 1 oz all layers | 0.5 oz base foil outer (1 oz finished), 1 oz inner |
| Finished thickness | 0.062 in nominal (62.4 mil stackup) | 0.062 in nominal (62.3 mil stackup) |
| Controlled impedance | 90 Ω ±10 % differential, layer 5, referenced to layers 4 and 6 | 90 Ω ±10 % differential, layer 1, referenced to layer 2 |

KiCad makes the stackup table and the impedance table on each fabrication drawing from the
`*_stackup.json` and `*_impedance.json` files in the KiCad project folder.

Each `_cam/` folder contains these files:

- Gerber X2 files for all copper, mask, paste, silkscreen and outline layers
- Excellon PTH and NPTH drill files, and a drill report
- An IPC-D-356 netlist (`.d356`)
- A KiCad XML netlist

Each `_stn/` folder contains the copper, paste and outline Gerber files for the stencil, and the
pick-and-place workbook. Each pick-and-place workbook has three sheets: `TopComp`, `BottomComp` and
`CombinedComp`.

## Platform card (`100064667`)

| Function | Device |
| --- | --- |
| Interface MCU | STM32C011F6 (Arm Cortex-M0+), `U5` |
| Payload power switch | MPQ5069, 15 A protection switch with current monitor, `U7` |
| Current sense | ACS711, ±15 A bidirectional Hall-effect with overcurrent fault output, `U8` |
| Ideal-diode / reverse-polarity control | NCV68261 driving an NTTFS2D1N04HL N-channel MOSFET, `U2` / `Q1` |
| Undervoltage supervisor | LTC2965, `U3` |
| Supply supervisor | MAX821, 2.78 V, `U11` |
| Detect / enable logic | SN74LVC1G08 AND gate `U12`, SN74AUP2G04 inverter `U13` |
| Logic 3.3 V rail | NCP730 LDO, 150 mA, `U4` |
| USB routing | TC7USB40MU dual SPDT `U9`, MAX20336 DPST `U10` |
| Transient protection | SMF26CA 26 V bidirectional TVS on `VBATT`, SMF3.3 on signals, PESD5V0C2UM on USB |
| Debug | Tag-Connect 6-pin SWD footprint, `J11` |

[`MFR_DNP.md`](MFR_DNP.md) and [`MFR_Rework.md`](MFR_Rework.md) remove `U3`, `R1` to `R4`, `C2`,
`JP1`, `U11` and `U12`. After you apply the two notes, the card has these changes:

- It has no undervoltage supervision.
- It has no supply supervisor.
- Its detect/enable gate is a permanent pass-through.

If your application needs brownout protection or a supervised power sequence, you must supply it.
The reference design does not supply it at this time.

## Payload card (`100068622`)

The payload card has the DS2431 1-Wire EEPROM. The platform MCU reads this EEPROM to identify the
payload. Thus, a payload that uses this card does not need its own identity device.

| Function | Device |
| --- | --- |
| V1 / V2 regulation | MAX25254, dual 36 V input / 0.8 to 14 V output synchronous buck, `U3`; XEL4030 0.47 µH inductors |
| Payload ID / detect | DS2431 1-Wire EEPROM, 1 kbit, `U4` |
| Input protection | NCV68261 driving an NTTFS2D1N04HL N-channel MOSFET, `U5` / `Q1` |
| Rail discharge | DMP610DL P-channel MOSFET, `Q2` |
| Logic 3.3 V rail | NCP730 LDO, 150 mA, `U2` |
| Supply supervisor | MAX821, 2.78 V, `U6` |
| Logic | SN74AUP2G04 inverter, `U7` |
| USB routing | TC7USB40MU dual SPDT `U8`, MAX20336 DPST `U9` |
| Transient protection | SMF26A 26 V unidirectional TVS on `VBATT`, SMF3.3 on signals, PESD5V0C2UM on USB |

The design does not install `C20` and `R14` (do not populate). The pick-and-place file does not
include them.

## Firmware

The platform MCU firmware is in [`sUPI Firmware/`](../sUPI%20Firmware). Its
[README](../sUPI%20Firmware/README.md) gives the operation of the payload detection, the current
monitor and the platform communication.

| Item | Detail |
| --- | --- |
| Target | STM32C011F6 (Arm Cortex-M0+) |
| Project | Keil MDK-ARM (`MDK-ARM/supi_firmware.uvprojx`), Arm Compiler 6 |
| Configuration | STM32CubeMX 6.8.0 (`supi_firmware.ioc`) |
| Application code | `Core/Src/supi.c`, `payload_power.c`, `stm32c0x1_one_wire.c` and their headers |
| Prebuilt image | `build/supi_firmware.hex`, Intel HEX, 20,440 bytes, load address `0x08000000` |

The ST HAL and CMSIS sources in `Drivers/` are vendor code. They have their own licenses.

## How to change the cards

The KiCad projects are complete, and you can change them. Their file format is KiCad 8. They
contain these items:

- Schematics and boards
- Stackup, impedance and fabrication-note data
- Project text variables for the title blocks

The design files contain the symbols and the footprints. The repository does not contain the vendor
3D models. The board files look for these models in `${SUPI_3DMODEL_DIR}`. Set this path in the
KiCad path configuration.

Send changes to the reference design as KiCad project files. Refer to
[`CONTRIBUTING.md`](../CONTRIBUTING.md).
