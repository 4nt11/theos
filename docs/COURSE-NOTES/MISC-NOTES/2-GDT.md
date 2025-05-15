# What the hell is the global descriptor table?
The global descriptor table is the table that contains all the segments of memory. It's accessed by the CPU to know where the programs and data is.

It can also be used to set protection in the RAM, as some access and flag bytes enable or disable execution or the ability to read those memory segments.
# Switching to protected mode.
This is how we access protected mode:
```
cli            ; disable interrupts
lgdt [gdtr]    ; load GDT register with start address of Global Descriptor Table
mov eax, cr0 
or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
mov cr0, eax

; Perform far jump to selector 08h (offset into GDT, pointing at a 32bit PM code segment descriptor) 
; to load CS with proper PM32 descriptor)
jmp 08h:PModeMain

PModeMain:
; load DS, ES, FS, GS, SS, ESP
; at this point, we're on protected mode :)
```
But we need to create a GDT or Global Descriptor Table.
> Since the following code is quite large, I'll set it up in pieces.

```
ORG 0x7c0
```
- Now we'll begin from 0x7c0 directly again.
```
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
```
- These we'll learn what they are in a few moments.
```
start:
	jmp 0:step2
```
- Now instead of setting 0 to 0x7c0, we'll set it to 0, since we're already at origin 0x7c0.
```
.load_protected:
        cli
        lgdt [gdt_descriptor]
        mov eax, cr0
        or eax, 0x1
        mov cr0, eax
        jmp CODE_SEG:load32
```
- The `cli` instruction clears and disables interrupts.
- `mov eax, cr0` moves the value of `eax` into the `cr0` register.
- `or eax, 0x1` we set `eax` to 0x1 to set the PE (Protection Enable) bit in `cr0` (Control Register 0)
- `mov cr0, eax` now we set the PE bit.
- `jmp CODE_SEG:load32` we'll see what this does in a bit.
More info on the following GDT code: https://wiki.osdev.org/GDT_Tutorial#Flat_/_Long_Mode_Setup
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
- `gdt_start:` just setting up the initial label for our GDT.
- `gdt_null:` this label points to null values, which are needed by the CPU to mark the end of the GDT.
	- `dd 0x0`: we're setting up null values.
	- `dd 0x0`: again, we're setting up null values.
- `gdt_code:` we begin the GDT code. The `cs` register should point to this. It will be used to store executable code.
	- `dw 0xffff`: This is the segment limit. It is set to 0xFFFF because the limit is 64KB. Although standard, this is an arbitrary limit.
	- `dw 0`: This is the base first 0-15 bits. For more info: https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
	- `db 0`: This is the base 16-23 bits. For more info: https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
	- `db 0x9a`: This is the access byte. It specifies the access rights and properties of the segment.
		- Types available in 32-bit protected mode:
			- **0x1:** 16-bit TSS (Available)
			- **0x2:** LDT
			- **0x3:** 16-bit TSS (Busy)
			- **0x9:** 32-bit TSS (Available)
			- **0xB:** 32-bit TSS (Busy)
		- Types available in Long Mode:
			- **0x2:** LDT
			- **0x9:** 64-bit TSS (Available)
			- **0xB:** 64-bit TSS (Busy)
	- `db 0x11001111b`: This is the flags byte. [Read this if you forget about it.](http://www.osdever.net/tutorials/view/the-world-of-protected-mode)
	- `db 0`: This is the base 24-31 bits.

**1st Double word:**

| **Bits** | **Function** | **Description**                      |
| -------- | ------------ | ------------------------------------ |
| 0-15     | Limit 0:15   | First 16 bits in the segment limiter |
| 16-31    | Base 0:15    | First 16 bits in the base address    |
**2nd Double word:**

| **Bits** | **Function**    | **Description**                                                      |
| -------- | --------------- | -------------------------------------------------------------------- |
| 0-7      | Base 16:23      | Bits 16-23 in the base address                                       |
| 8-12     | Type            | Segment type and attributes                                          |
| 13-14    | Privilege Level | 0 = Highest privilege (OS), 3 = Lowest privilege (User applications) |
| 15       | Present flag    | Set to 1 if segment is present                                       |
| 16-19    | Limit 16:19     | Bits 16-19 in the segment limiter                                    |
| 20-22    | Attributes      | Different attributes, depending on the segment type                  |
| 23       | Granularity     | Used together with the limiter, to determine the size of the segment |
| 24-31    | Base 24:31      | The last 24-31 bits in the base address                              |
- `gdt_descriptor`: This is the GDT descriptor label.
	- `dw gdt_end - gdt_start-1`: This is the size of the GDT in bytes minus 1, as per CPU requirements.
        - `dd gdt_start`: This is the address of the GDT in memory.
**The GDT Descriptor**

| **Bits** | **Function** | **Description**      |
| -------- | ------------ | -------------------- |
| 0-15     | Limit        | Size of GDT in bytes |
| 16-47    | Address      | GDT's memory address |

```
[BITS 32]
load32:
        mov ax, DATA_SEG
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov ebp, 0x00200000
        mov esp, ebp
        jmp $
```
- `[BITS 32]`: this sets the assembler to use 32 bit instructions and registers.
- `load32:` This is a label that will be called when we load into protected mode.
	- `mov ax, DATA_SEG`: We move `DATA_SEG` into `ax`. Remember that `DATA_SEG` we saw before? `DATA_SEG equ gdt_data - gdt_start-1`? This is why it's here. It makes the `ds` register point to offset `0x10`.
	- `mov ds, ax`: Now we set all our segments to the `0x10` offset or `DATA_SEG` of our GDT.
	- `mov es, ax`: Same here.
	- `mov fs, ax`: Same here.
	- `mov gs, ax`: Same here.
	- `mov ss, ax`: Same here.
	- `mov ebp, 0x00200000`: Here we're using the 32 bit `ebp` register which is the base pointer. We're setting it to `0x00200000`, which is a 32 bit number.
	- `mov esp, ebp`: Now we're setting the stack pointer to point to the base pointer, initializing our memory.
	- `jmp $`: We do nothing else.

Pretty fucking complex right? So fun!!!
