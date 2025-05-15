# Description
This page details some information on the current state of the OS bootloader. 
# Related code
The related code can be found on the following directories:
- src/boot
- src/boot.asm
# Architecture
The bootloader it's not smart. At all. All it knows is that the following sector in the disk contains the kernel. Nevertheless, it has some important aspects to keep in mind.
## GDT Code Segments
We define two important segments in the beginning of the `boot.asm` file: `CODE_SEG` and `DATA_SEG`. Both do important stuff later on when the GDT is loaded and the kernel is running.
```
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
```
## Boot Parameter Block
At the beginning of the `boot.asm` file, we can find the Boot Parameter Block. Although it is a dummy BPB, it is a BPB nonetheless.
```
_start:
        jmp short start
        nop
times 33 db 0
```
## Global Descriptor Table
We need to create a global descriptor table descriptor and send it to the CPU at boot time. This is done through the `gdt_*` labels found in `boot.asm`:
```
; GDT!
gdt_start:
gdt_null:
        dd 0x0
        dd 0x0

; offset 0x8
gdt_code:               ; cs should point to this.
        dw 0xffff       ; segment limit first 0-15 bits
        dw 0            ; base first 0-15 bits.
        db 0            ; base 16-23 bits.
        db 0x9a         ; access byte
        db 11001111b    ; high 4 bit flags and low 4 bit flags.
        db 0            ; base 24-31 bits.

; offset 0x10
gdt_data:               ; linked to DS, SS, ES, FS, GS
        dw 0xffff       ; segment limit first 0-15 bits
        dw 0            ; base first 0-15 bits.
        db 0            ; base 16-23 bits.
        db 0x92         ; access byte
        db 11001111b    ; high 4 bit flags and low 4 bit flags.
        db 0            ; base 24-31 bits.
gdt_end:

gdt_descriptor:
        dw gdt_end - gdt_start-1
        dd gdt_start
```
And in the label `load32`, there's the `lgdt` call.
```
.load_protected:
        cli
        lgdt [gdt_descriptor] ; this is it.
        mov eax, cr0
        or eax, 0x1
        mov cr0, eax
        jmp CODE_SEG:load32
```
## Jump to 32 bit Protected Mode
Jumping to the 32 bit protected mode is done right after loading our GDT in the `.laod_protected` label.
```
.load_protected:
        cli
        lgdt [gdt_descriptor]
        mov eax, cr0
        or eax, 0x1
        mov cr0, eax
        jmp CODE_SEG:load32
```
And the first bits of code executed are the `load32` label.
```
[BITS 32]
load32:
        mov eax, 1
        mov ecx, 100
        mov edi, 0x0100000
        call ata_lba_read
        jmp CODE_SEG:0x0100000
```
Which loads our kernel via the ATA LBA driver and then jumps into it.
## LBA Read Driver.
Before jumping into our kernel, we need a driver to read from the disk and actually load it into an accesible place in memory. This is done through the ATA LBA driver contained within our bootloader. The code is quite extensive and it's documented in the [[LBA ATA Related notes]] page.
```
ata_lba_read:
        mov ebx, eax ; backup the lba
        ; send the highest 8 bits to the hard disk controller
        shr eax, 24
        or eax, 0xE0 ; selects the master drive
        mov dx, 0x1F6
        out dx, al
        ; finish sending the highest 8 bits of the lba

        ; send the total sectors to read
        mov eax, ecx
        mov dx, 0x1F2
        out dx, al
        ; done sending the sectors to read

        mov eax, ebx ; restoring the lba backup
        mov dx, 0x1F3
        out dx, al
        ; finished sending more bits of the LBA

        mov dx, 0x1F4
        mov eax, ebx ; restoring again the LBA backup
        shr eax, 8
        out dx, al
        ; finished sending even more bits of the LBA

        ; send uppter 16 bits
        mov dx, 0x1F5
        mov eax, ebx ; restoring the LBA backup
        shr eax, 16
        out dx, al

        ; finished sending upper 16 bits of the LBA
        mov dx, 0x1F7
        mov al, 0x20
        out dx, al

        ; read all sectors into memory
.next_sector:
        push ecx

; checking if we need to read
.try_again:
        mov dx, 0x1F7
        in al, dx
        test al, 8
        jz .try_again
; we need to read 256 words at a time
        mov ecx, 256
        mov dx, 0x1F0
        rep insw
        pop ecx
        loop .next_sector
        ; end of reading sectors into memory
        ret
```
Nevertheless, the `shr` instructions are calculating the LBA address (`LBA address = (ecx >> 24) + (ecx >> 16) + (ecx >> 8)`) and then reading after the `0x1F7` command is send to the I/O port. After that, it loops using the `.try_again` and `.next_sector` labels.
## Kernel execution.
After we load the kernel into memory using the LBA driver, we can execute it using the `jmp` instruction. This is done in the `laod32` label.
```
[BITS 32]
load32:
        mov eax, 1
        mov ecx, 100
        mov edi, 0x0100000
        call ata_lba_read
        jmp CODE_SEG:0x0100000
```

