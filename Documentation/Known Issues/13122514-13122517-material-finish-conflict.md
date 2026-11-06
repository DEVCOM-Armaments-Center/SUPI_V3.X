<!-- Title: Drawings 13122514 and 13122517: the hard anodize finish cannot apply to the polymer material option -->

`19200_13122514` (V-Mount Base) and `19200_13122517` (Connector Cover) permit two materials: 6061-T6 aluminium or polymer powder bed fusion. Both drawings also specify hard anodize per MIL-A-8625F Type III Class 2. Hard anodize applies only to aluminium.

`19200_13122517` also has a second conflict. Note 2 says to use additive manufacturing, but note 3 permits aluminium.

### Why this is a problem

A shop that makes a polymer part cannot apply the specified finish. A shop that makes `13122517` from aluminium does not obey note 2.

### Workaround

Apply the specified finish only to aluminium parts.

### Fix

- [ ] On both drawings, change the finish note so that it applies only to the aluminium option.
- [ ] Give a finish for the polymer option (for example, the depowder and media blast process on `13122516`).
- [ ] On `19200_13122517`, make note 2 and note 3 agree.

### Check

- [ ] Each material option on each drawing has a finish that applies to it.
