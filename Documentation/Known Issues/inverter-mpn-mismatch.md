<!-- Title: Platform U13 and payload U7: the inverter MPN does not agree with the value and footprint -->

The inverter on both cards has two different part numbers in the design data:

| Card | Ref | Value | MPN in the schematic and BOM | Footprint |
|---|---|---|---|---|
| Platform `100064667` | `U13` | `SN74AUP2G04DRYR` | `SN74AUP3G04DQER` | `QFN-6_1.4x1mm_P0.5mm TI DRY6` |
| Payload `100068622` | `U7` | `SN74AUP2G04DRYR` | `SN74AUP3G04DQER` | `QFN-6_1.4x1mm_P0.5mm TI DRY6` |

The BOM description is "IC INVERTER 3CH 3-INP 8X2SON", which matches the MPN. The value and the footprint are for `SN74AUP2G04DRYR`, a dual inverter in the 6-pin DRY package. `SN74AUP3G04DQER` is a triple inverter in the 8-pin DQE package.

### Why this is a problem

A buyer who orders from the MPN column gets an 8-pin part. The 8-pin part does not fit the 6-pin footprint on the board.

### Workaround

Order `SN74AUP2G04DRYR` for platform `U13` and payload `U7`. It agrees with the value, the footprint and the schematic symbol.

### Fix

- [ ] In both schematics, change the `MPN` and description fields of the inverter to `SN74AUP2G04DRYR`.
- [ ] Generate both BOMs again.

### Check

- [ ] The value, MPN, description and footprint of `U13` and `U7` all identify `SN74AUP2G04DRYR`.
