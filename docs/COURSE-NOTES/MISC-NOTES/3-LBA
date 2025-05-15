file:///C:/Users/b0ss/Downloads/d1532v1r4b-ATA-ATAPI-7-1.pdf
- Page 88: Command descriptions.
	- Page 234: READ SECTOR(S).
file:///C:/Users/b0ss/Downloads/d1532v2r4b-ATA-ATAPI-7-2.pdf
file:///C:/Users/b0ss/Downloads/d1532v3r4b-ATA-ATAPI-7-3.pdf

>`LBA address = (ecx >> 24) + (ecx >> 16) + (ecx >> 8)`
# Meaning of LBA.
LBA means Logical Block Addressing. It's the newer version of CHS or "Cylinder-Head-Sector". [LBA on Wikipedia](https://en.wikipedia.org/wiki/Logical_block_addressing)

# LBA ports
- `0x1F6`: This is the port address for the ATA drive's control register. Specifically, it's used to select the drive and set the LBA mode.
- `0x1F2`: This is the port address for the ATA drive's sector count register. It's used to specify the number of sectors to read or write.
- `0x1F3`: This is the port address for the ATA drive's LBA low register. It's used to send the lower 8 bits of the LBA address to the drive.
- `0x1F4`: This is the port address for the ATA drive's LBA mid register. It's used to send the middle 8 bits of the LBA address to the drive.
- `0x1F5`: This is the port address for the ATA drive's LBA high register. It's used to send the upper 8 bits of the LBA address to the drive.
- `0x1F7`: This is the port address for the ATA drive's command register. It's used to send commands to the drive, such as "read sector" or "write sector".
	- `0x20`: Read sectors (with retry)
		- p. 234 ATA-7 Specification.
	- `0x30`: Write sectors (with retry)
		- p. 367 ATA-7 Specification.
	- `0xC0`: Delete sectors
		- p. 90 ATA-7 Specification.
	- `0x1H`: Identify packet device (for ATAPI devices)
		- p. 159 ATA-7 Specification.
	- `0x90`: Check power mode
	- `0xE5`: Standby immediate
	- `0x95`: Idle immediate
	- `0xB0`: Reset device
- `0x1F0`: This is the port address for the ATA drive's data register. It's used to read or write data from the drive.

# Master and slave addresses
- 0xE0: address for the master drive.
- 0xF0: address for the slave drive.

