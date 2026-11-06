<!-- Title: Drawings 13122514 and 13122517: note 5 names a superseded design model -->

Note 5 on each detail drawing names the design model that completes the product definition. Two drawings name a superseded model:

| Drawing | Note 5 cites | Current model |
|---|---|---|
| `19200_13122514` | `100051078` | `100068609` |
| `19200_13122517` | `100051078` | `100068617` |
| `19200_13122515` | `100064656` | correct |
| `19200_13122516` | `100051265` | correct |
| `19200_13122518` | `100064665` | correct |

### Why this is a problem

Undimensioned features come from the design model. A shop that follows note 5 on these two drawings uses the wrong model for those features.

The design models are not in this release. A reader cannot compare them to find the difference.

### Fix

- [ ] On `19200_13122514`, change note 5 to `100068609`.
- [ ] On `19200_13122517`, change note 5 to `100068617`.
- [ ] Release the two drawings again as PDF.

### Check

- [ ] Note 5 on all five detail drawings agrees with the table above.
