# ndisasm !

# Important pieces of information:
> 0x7c00: Where the bootloader is placed by the BIOS.
> 0x55AA: Boot signature (at the 511th and 512th byte) the BIOS looks for when trying to boot up the bootloader.

# What is memory?
- Piece of hardware that allow computers to store information.
- RAM (Random Access Memory) is the main memory, where programs read and write information.
- ROM (Read-only Memory) is a form of memory that can only be read from.

# RAM.
- Temporary.
- Writeable.
- RAM is not persistent.

# ROM
- Permanent.
- Can't be written to normally.
- Persistent.

![[Pasted image 20250429224017.png]]
BIOS Chip (ROM).

# Boot process.
- The BIOS is executed directly from ROM.
- The BIOS loads the bootloader into 0x7c00.
- The bootloader loads the kernel.

# Bootloader.
- Small program responsible for loading the kernel of an operating system.
- Generally small.

# When booting...
- The CPU executes instructions directly from the BIOS's ROM.
- The BIOS generally loads itself into RAM then continues execution from RAM.
- The BIOS will initialize essential hardware.
- The BIOS looks for a bootloader to boot by searching all storage mediums for the boot signature 0x55AA at the 511th and 512th byte.
	- If the sector is found, the sector will be loaded into 0x7c00 and execute from there.
	- A sector is just a block of storage, in hard drives them being 512 bytes.
- When the bootloader is loaded, the BIOS will do an absolute jump to the address 0x7c00 and start the operating system boot process via the bootloader.
- If the BIOS can't find a bootable sector, it can't do anything more.

# The BIOS is almost a kernel by itself.

- The BIOS contains routines to asssit our bootloader in booting our kernel.
- The BIOS is 16 bit code which means only 16 bit code can execute it properly.
- BIOS routines are generic and standard (more later).

# Setup.
- Install NASM (nasm).
- Install QEMU (qemu-system-x86)
