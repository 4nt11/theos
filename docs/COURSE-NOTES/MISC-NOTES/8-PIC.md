# Standard ISA IRQ's

| IRQ | Description                                             |
| --- | ------------------------------------------------------- |
| 0   | Timer interrupt.                                        |
| 1   | Keyboard interrupt.                                     |
| 2   | Cascase (used internally by the two PICs. never raised) |
| 3   | COM2 (if enabled)                                       |
| 4   | COM1 (if enabled)                                       |
| 5   | LPT2 (if enabled)                                       |
| 6   | Floppy Disk                                             |
| 7   | LPT1                                                    |
| 8   | CMOS real time clock (if enabled)                       |
| 9   | Free for peripherals/legacy SCSI/NIC                    |
| 10  | Free for peripherals/SCSI/NIC                           |
| 11  | Free for peripherals/SCSI/NIC                           |
| 12  | PS2 Mouse                                               |
| 13  | FPU/Coprocessor/Inter-processor                         |
| 14  | Primary ATA hard disk                                   |
| 15  | Secondary ATA hard disk                                 |
# Ports
- `0x20` and `0x21` = Master IRQs.
- `0xA0` and `0xA1` = Slave IRQs.
