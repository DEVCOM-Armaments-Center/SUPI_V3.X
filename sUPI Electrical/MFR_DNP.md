# sUPI Electrical Interfaces — Do Not Populate

**DEPARTMENT OF THE ARMY**
U.S. Army Combat Capabilities Development Command, Armaments Center
Picatinny Arsenal, New Jersey 07806-5000

| | |
| --- | --- |
| **Office Symbol** | FCDD-ACE-P |
| **Date** | 30 March 2026 |
| **Subject** | sUPI Electrical Interfaces Do Not Populate |
| **Applies to** | Platform card `PB-F-0210` (assembly `100064667`) |

> **DISTRIBUTION STATEMENT A. APPROVED FOR PUBLIC RELEASE; DISTRIBUTION IS UNLIMITED.**

---

## Direction

The low-battery / undervoltage cutoff feature of the sUPI reference design is under development and
in its current form can prevent power being supplied to sUPI payloads.

Unless the feature is specifically required, **do not populate** the following components on the
**platform** board. Omitting them is also a cost saving.

| Reference | Device | Function |
| --- | --- | --- |
| `U3` | LTC2965HDD | Undervoltage supervisor |
| `R1` | 787 kΩ, 1%, 0402 | Undervoltage threshold divider |
| `R2` | 12.4 kΩ, 0.1%, 0402 | Undervoltage threshold divider |
| `R3` | 200 kΩ, 0.1%, 0402 | Undervoltage threshold divider |
| `R4` | 1 kΩ, 1%, 0402 | Supervisor support |
| `C2` | 0.1 µF, 50 V, 0402 | Supervisor decoupling |
| `JP1` | — | Low-battery cutoff jumper |

**Effect:** the platform board is built without undervoltage lockout, so payload power continues to
be supplied as the battery falls. If your application depends on brownout protection, implement it
in platform avionics upstream of this interface.

---

**Respectfully submitted,**

Robert Barone Jr, PMP \
Special Project APO \
U.S. Army Combat Capabilities Development Command, Armaments Center \
Picatinny Arsenal, NJ 07806 \
W: 520-684-1640 \

