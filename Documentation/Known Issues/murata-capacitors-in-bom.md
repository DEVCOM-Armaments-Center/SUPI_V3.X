<!-- Title: Both cards: the released BOMs still use Murata capacitors -->

[`MFR_Capacitors.md`](../MFR_Capacitors.md) gives non-Murata alternates for the capacitors on both cards. The reason is Murata's published policy against the use of its products in weapon systems.

The KiCad BOMs in this release still use the Murata parts:

| Card | Murata positions | Count |
|---|---|---|
| Payload `100068622` | `C1` to `C19`, `C21` to `C24` | 23 |
| Platform `100064667` | `C1` to `C14`, `C18`, `C23`, `C24`, `C27` to `C33` | 24 |

`MFR_Capacitors.md` has an alternate for each of these positions.

### Why this is a problem

An assembler who builds from the BOM orders Murata parts, against the direction in `MFR_Capacitors.md`.

### Workaround

Use the alternate MPNs in `MFR_Capacitors.md`. Values, dielectrics, voltage ratings and packages are the same, except platform `C10`: the alternate is X7S, not X5R.

### Fix

- [ ] In both schematics, change the MPN and manufacturer fields of each Murata capacitor to the alternate in `MFR_Capacitors.md`.
- [ ] Generate both BOMs again.

### Check

- [ ] No BOM line has manufacturer `MURATA ELECTRONICS`.
