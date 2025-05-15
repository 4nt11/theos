# 1. Writting a Hello World bootloader.
## The code
```
ORG 0x7c00 ; 
BITS 16

start:
	mov ah, 0eh
	mov al, 'A'
	mov bx, 0
	int 0x10

	jmp $

times 510-($ - $$ ) db 0
dw 0xAA55
```

## ORG 0x7c00
- This line instructs the assembler to load the binary into address 0x7c00
## BITS 16
- This instructs the assembler to only assembly instructions into 16 bit assembly code.
## start
- This is a label to represent where we'll write our code.
## mov ah, 0eh
- 0eh or teletype output is a way to write characters in the screen.
## mov al, 'a'
- The register `al` is a register that will be written on screen.
## mov bx, 0
- This sets the background color to black.
## int 0x10
- Executes the 0eh call (teletype output).
## times 510-(\$ - $\$) db 0
- This tells the assembler to fill space with 510 bytes of data.
	- times: the assembly directive that tells the assembler to repeat an instruction a given amount of times.
	- 510: is the amount of bytes that we want to fill,
	- $: is the current address of the code,
	- \$$: represents the starting address of the current section or segment in the code.
	- ($ - $\$): calculates the offset from the start of the section to the current address. This gives the number of bytes that have been used so far.
	- 510-($ - $\$): calculates the difference between 510 and the number of bytes used so far. This gives the number of bytes that still need to be filled to reach a total of 510 bytes.
	- `db 0`: This is the instruction that is being repeated. `db` stands for "define byte", and it tells the assembler to reserve space for a byte and initialize it with the given value. In this case, the value is `0`, so the assembler will fill the specified number of bytes with zeros.
## dw 0xAA55
- This puts the byte 0x55AA in the last two bytes. It's reversed in the assembly, as we're working with Little Endian architecture, where the most significant bits are flipped when working with words.
## Compilation
```
nasm -f bin boot.asm -o boot.bin
```
## Why BIN?
- The BIOS has no concept of libraries, ELFs, or EXEs. It needs to be raw binary. It's a file without headers. Just raw code.

## New code.
```
ORG 0x7c00
BITS 16

start:
	mov si, message
	call print
	jmp $

print:
	mov bx, 0
	
.loop
	lodsb
	cmp al, 0
	je .done
	call print_char
	jmp .loop
	
.done: 
	ret

print_char:
	mov ah, 0eh
	int 0x10
	ret

message: db 'hola cybeer y resaca :)', 0

times 510-($-$$) db 0
dw 0xAA55
```

## mov si, message
- Move the memory address of message into the `si` register.
### si IS NOT stack index.
- By the way: the `si` register IS NOT the stack index. It's the source index, used to store the offset address of a source opearnd in string manipulation operations (what we're doing with `mov ah, 0eh`.)
## call print
- This will call the print function.
## print:
- Just another label. In this case, we're defining a subroutine.
## mov bx, 0
- As before, we're setting the background color.
## .loop:
- This is a subsubroutine only accesible to the `print` subroutine. In the code, it's used to create a loop to print the characters.
## lodsb
- The `lodsb` directive loads the value of the `si` register into the `al` register and increment it. That's why we're not moving data into `al`, because `si` points to `message`.
- For example:
	- If `si` points to the memory section where `message` ("Hello world") is stored, the `lodsb` directive will put the **H** character in the `al` register and increment it, meaning that after the initial `lodsb` instruction, `si` will now point to **e** instead of **H**, and so on and so forth.
	- Remember: the `si` register DOES NOT contain the ENTIRE message. It stores the MEMORY VALUE of where message is at, beginning at the first character of the message itself. I will repeat: **IT DOES NOT CONTAIN THE ENTIRE MESSAGE!**.
## cmp al, 0
- This instruction compares the value of `al` with 0. This is because or message is terminated with a zero (or null byte). Depending on whether the comparison is true or false, it will...
## je .done
- ...jump to the .done subsubroutine. It's like an `if`, although in this case it's more like a while, since:
```
while si != "0";
do
	print_char($si)
	si=$si+1
done
```
- But the logic is the same :)
## call print_char
- If `cmp al, 0` is false, the subrountine `print_char` will be called.
## jmp .loop
- After calling `print_char`, the program will go to the `.loop` subsubroutine, rerunning the `print` subroutine. Like a while loop :)
## .done
- Again, just a subsubroutine. It will be accessed only when `si` is equal to **0**.
## ret
- Returns to the calling function.

# 2. Understanding Real Mode.
- We have 1 megabyte of RAM accessible.
	- Memory is accessible through the use of segments.
	- It does not matter if you have more than 1 megabyte of RAM. The realmode compatibility layer acts as an older 8086 CPU.
- Based on the original 8086 design.
	- All the code MUST be 16 bits
- It has no security at all.
	- No memory security.
	- No hardware security.
	- Simple user programs can destroy our operating system with no way for us to stop them.
- We only have 16 bits accessible at all time.
	- We can access 8 and 16 bit registers.
	- It can be overwritten by a per-instruction basis, but thats a no go.
	- We can only request memory address offsets of up to 65535 for a given segment.
	- We can use only 16 bit numbers.
# 3. Segmentation Memory Model
# 3. IMPORTANT FORMULA: ss * 16 + sp
- Memory is accessed by a segment and an offset.
- Programs can be oaded in different areas of memory but run without problems.
- Multiple segments are available through the use of segment registers.
## 8086 Segment Registers
- `cs` - Code segment.
- `ss` - Stack segment.
- `ds` - Data segment.
- `es` - Extra segment.
## Calculating absolute offset.
- Take the segment register and multiply it by 16 and add the offset.
	- The offset is the absolute position in RAM that a particular byte is add.
		- Code Segment = 0x7C0
		- Assembly `org` is zero.
		- Our first instruction is at origin ero so our offset is zero.
			- (0x7C0 * 16) = 0x7C00
			- 0x7C00 + 0 = 0x7C00
				- Where 0 is the offset (origin).
### Offset examples for absolute address 0x7CFF
- Segment 0 offset 0x7CFF
- Segment 0x7C0 offset 0xFF
- Segment 0x7CF offset 0x0F
	- Calculation would be:
		- 0x7CF * 16 = 0x7CF0
		- 0x7CF0 + 0x0F = 0x7CFF
## Different instructions use different segment registers.
- `lodsb` uses the `ds:si` register combination.
### Code example:

```
ORG 0

mov ax, 0x7c0
mov ds, ax
mov si, 0x1F
lodsb
```
And...
0x7C0 * 16 = 0x7C00
0x7C00 + 0x1F = 0x7C1F
![[Pasted image 20250430121649.png]]

## Program can be loaded into different areas of memory and run without problems.
- Imagine we have two programs, both with origin at 0.
- Program 1 uses segment 0x7C0 for all its segment registers.
- Program 1 is loaded at address 0x7C00.
- Program 2 uses segment 0x7D0.
- Program 2 is loaded at address 0x7D00.
- We swap segment registers when switching to the other process.
- We restore all the registers of the process we are switching to.
- It resumes as if we never switched processes at all.
## Multiple segments are available through the use of segment registers.
- `mov byte al, [es:32]`
	- Extra segment, the absolute offset in RAM that it'll get one byte of data will be the extra segment * 16 + 32d.
- `mov byte al, [ds:826]`
	- Data segment, the absolute offset in RAM that it'll get one byte of data will be the extra segment * 16 + 826d.
- `mov byte al, [ss:231]`
	- Stack segment, the absolute offset in RAM that it'll get one byte of data will be the extra segment * 16 + 231d.
## The stack segment.
- `ss` (stack segment) = 0x00.
- `sp` (stack pointer) = 0x7C00.
- `push 0xFFFF`: It decrements the stack pointer by 2.
	- That's because 0xFFFF is a two byte hex value. Meaning that we're storing a word, that why we're decrementing it by 2.
	- 2 bytes = word.
- Meaning that now, our `sp` turned from 0x7C00 into: `sp` = 0x7BFE.
	- because 0x7C00 - 2 = 0x7BFE.
- The combination of the stack segment and the stack pointer (0x7C00), when we push it becomes 0x7BFE.
	- That's because, again, a word is two bytes. We're doing:
		- 0x7C00 - 2 = 0x7BFE + 1 = 0x7BFF.
		- So our word is stored at memory address 0x7BFE and 0x7BFF.
			- 0x7BFE contains our low byte (0xFF),
			- 0x7BFF contains our high byte (0xFF).
### AI Help (by Llama)
Prompt: I'm studying this topic further, now getting into the segmentation memory model. I have a question regarding the `sp` and `ss` registers. See, I have the following statements: 
- `ss` (stack segment) = 0x00. 
- `sp` (stack pointer) = 0x7C00. 
- `push 0xFFFF`: It decrements the stack pointer by 2. 
- Meaning that now, our `sp`: `sp` = 0x7BFE. 
- The combination of the stack segment and the stack pointer (0x7C00), when we push it becomes 0x7BFE. 
- We then set the bytes 0x7BFE and 0x7BFF to 0xFFFF, because we push our word into it. 

Why is this? Where does 0x7BFF comes from? I understand that 0x7C00 - 2d = 0x7BFE, but I don't understand where that 0x7BFF is coming from! Please help!

Answer:

Let's break down what's happening when you push a word onto the stack:

1. **Stack Segment (`ss`)**: This register stores the segment address of the stack. In your case, `ss = 0x0000`.
2. **Stack Pointer (`sp`)**: This register stores the offset within the stack segment where the next item will be pushed or popped. Initially, `sp = 0x7C00`.

When you push a word (2 bytes) onto the stack, here's what happens:

- The stack pointer is decremented by 2 (because you're pushing a word, which is 2 bytes long). This means `sp` becomes `0x7C00 - 2 = 0x7BFE`.
- The word being pushed is stored at the new stack pointer location. Since `sp` is now `0x7BFE`, the word will be stored at `0x7BFE` and `0x7BFF`.

The reason you're storing the word at both `0x7BFE` and `0x7BFF` is because of how memory addressing works in the 8086 architecture:

- The stack segment (`ss`) is used as the base address for the stack. In your case, `ss = 0x0000`.
- The stack pointer (`sp`) is used as an offset within the stack segment. When you access memory using `ss` and `sp`, the address is calculated as `ss * 16 + sp`.
- Since `ss = 0x0000`, the base address of the stack is `0x0000 * 16 = 0x0000`.
- When `sp = 0x7BFE`, the address of the top of the stack is `0x0000 * 16 + 0x7BFE = 0x7BFE`.
- Since you're storing a word (2 bytes) at this location, the second byte of the word will be stored at the next address, which is `0x7BFF`.

So, to summarize:

- `0x7BFE` comes from decrementing the stack pointer (`sp`) by 2.
- `0x7BFF` comes from the fact that you're storing a word (2 bytes) at the location pointed to by `sp`, and the second byte of the word needs to be stored at the next address.

The calculation `ss * 16 + sp` is used to convert the segment and offset into a linear address. In your case, `ss = 0x0000` and `sp = 0x7BFE`, so the linear address is `0x0000 * 16 + 0x7BFE = 0x7BFE`. When you store a word at this location, the second byte will be stored at `0x7BFE + 1 = 0x7BFF`.

# 3. Improving our bootloader.
```
; modified origin
ORG 0
BITS 16

; new code
jmp 0x7c0:start
; end new code before start: label.
start:
	; new code
	cli ; clean interrupts.
	mov ax, 0x7c0
	mov ds, ax
	mov es, ax
	mov ax, 0x00
	mov ss, ax
	mov sp, 0x7c00
	
	;; for example purposes, not in the final code
	mov si, 20
	lodsb
	; what lodsb is doing is:
	; DS:SI, meaning that:
	; ds * 16 + si, or:
	; 0x7C0 * 16 = 0x7C00
	; 0x7C00 + 20 = 0x7C14
	; we do this because we do not know what the ds or si registers contain before we execute our program.
	; we need to initialize them, as we do with mov ds, ax and mov es, ax. 
	; if we dont do this, we're just hoping that the BIOS sets the registers as we need them, but if
	; we do it manually, we're taking control of things and making it correct every single time.
	;; end of example code
	sti ; enable interrupts.
	; end of new code
	mov si, message
	call print
	jmp $

print:
	mov bx, 0
.loop:
	lodsb
	cmp al, 0
	je .done
	call print_char
	jmp .loop
.done:
	ret

print_char:
	mov ah, 0eh
	int 0x10
	ret

message: db 'Hola amigos.', 0

times 510-($-$$) db 0
dw 0xAA55
```
## jmp 0x7c0:start
- This instruction will ensure that the start: label will be at 0x7C0.
## cli ; clean and disable interrupts.
- This instruction clear all the interrupts. This will guarantee that our program will do only what our program is trying to do, and not execute any other interrupts set before by the BIOS.
- This will also disable interrupts.
## mov ax, 0x7c0
- Now we're setting this value to later on assign it to the data segment and the extra segment registers. This is how the CPU works; we cannot assign 0x7C0 directly to the `ds` and `es` registers.
## mox ds, ax
- Now we're setting the data segment to `ax`, or 0x7C0.
- This is part of the stack initialization.
## mov es, ax
- Now we're setting the extra segment to `ax`, or 0x7C0.
- This is part of the stack initialization.
## mov ax, 0x00
- Now we set `ax` to 0x00, because...
## mov ss, ax
- ...we'll set the stack segment to 0x00. If we remember correctly, when setting up the stack, `ss * 16 + sp` is the absolute memory segment.
## mov sp, 0x7c00
- Now we assign 0x7C00 to the stack pointer, making our origin, effectively the 0x7C00 address, which is what we need.
## sti ; enable interrupts.
- Now we enable interrupts, because we will need them :)
## Compilation
```
# as always...
nasm -f bin boot.asm -o boot.bin
```
### AI Help.
Prompt:
```
My issue now is with the initialization itself. 
- Why do we have to set the `es` register? 
- Is `si` like the stack index on modern CPU architecture? 
- What does the `es` register do? 
- What would happen if instead of `sp` pointing to 0x7C00, it would be the `ss` register pointing to 0x7C00 instead? 

Thanks!
```
Answer:
```
Let's address each of your questions:

### 1. Why set the `es` register?

In this code, the `es` register is set to `0x7c0`, which is the same value as the `ds` register. This is because the `es` register is used as the destination segment for string operations, such as `lodsb` (load string byte) and `stosb` (store string byte).

In this specific code, the `es` register is not actually being used, since we're not performing any string operations that involve the `es` register. However, it's a good practice to initialize the `es` register to the same value as the `ds` register to avoid any potential issues with string operations.

### 2. Is `si` like the stack index on modern CPU architecture?

No, `si` is not like the stack index on modern CPU architecture. In x86 assembly language, `si` is the Source Index register, which is used as an index register for string operations.

In the given code, `si` is used to point to the `message` string, and the `lodsb` instruction loads a byte from the memory location pointed to by `ds:si` into the `al` register.

In modern CPU architectures, the stack index is typically implemented using a stack pointer register (such as `rsp` in x86-64) and a base pointer register (such as `rbp` in x86-64). The `si` register in x86 assembly language is not directly related to the stack index.

### 3. What does the `es` register do?

The `es` register is the Extra Segment register, which is used as the destination segment for string operations, such as `lodsb` and `stosb`. It's also used as the segment register for some instructions, such as `movsb` (move string byte) and `cmpsb` (compare string byte).

In general, the `es` register is used to specify the destination segment for data transfer operations, while the `ds` register is used to specify the source segment.

### 4. What if `ss` pointed to 0x7C00 instead of `sp`?

If `ss` pointed to 0x7C00 instead of `sp`, it would mean that the stack segment would start at address 0x7C00. This would be incorrect, because the stack segment should start at a lower address, and the stack pointer (`sp`) should point to the top of the stack.

In the given code, `ss` is set to 0x00, which means the stack segment starts at address 0x0000. The stack pointer (`sp`) is then set to 0x7C00, which means the top of the stack is at address 0x7C00.

If `ss` pointed to 0x7C00, it would cause the stack to grow downwards from address 0x7C00, which would overlap with the code segment and potentially cause stack corruption or other issues.

To illustrate the difference:

- `ss = 0x00`, `sp = 0x7C00`: Stack segment starts at 0x0000, stack pointer points to 0x7C00 (top of stack)
- `ss = 0x7C0`, `sp = 0x00`: Stack segment starts at 0x7C00, stack pointer points to 0x0000 (bottom of stack)

The first scenario is correct, while the second scenario would cause issues with stack management.
```
# 4. Preparing out bootloader for real hardware.
## BIOS Parameter Block ([more info](https://wiki.osdev.org/FAT#BPB_(BIOS_Parameter_Block)))
Some BIOS expect the BIOS Parameter Block. If we don't create it, the BIOS might overwrite the values of our program.
#### When booting from a USB, we're doing USB emulation, meaning that we're treating our USB as a hard drive, allowing us to talk to it.

|Offset (decimal)|Offset (hex)|Size (in bytes)|Meaning|
|---|---|---|---|
|0|0x00|3|The first three bytes EB 3C 90 disassemble to JMP SHORT 3C NOP. (The 3C value may be different.) The reason for this is to jump over the disk format information (the BPB and EBPB). Since the first sector of the disk is loaded into ram at location 0x0000:0x7c00 and executed, without this jump, the processor would attempt to execute data that isn't code. Even for non-bootable volumes, code matching this pattern (or using the E9 jump opcode) is required to be present by both Windows and OS X. To fulfil this requirement, an infinite loop can be placed here with the bytes EB FE 90.|
|3|0x03|8|OEM identifier. The first 8 Bytes (3 - 10) is the version of DOS being used. The next eight Bytes 29 3A 63 7E 2D 49 48 and 43 read out the name of the version. The official FAT Specification from Microsoft says that this field is really meaningless and is ignored by MS FAT Drivers, however it does recommend the value "MSWIN4.1" as some 3rd party drivers supposedly check it and expect it to have that value. Older versions of dos also report MSDOS5.1, linux-formatted floppy will likely to carry "mkdosfs" here, and FreeDOS formatted disks have been observed to have "FRDOS5.1" here. If the string is less than 8 bytes, it is padded with spaces.|
|11|0x0B|2|The number of Bytes per sector (remember, all numbers are in the little-endian format).|
|13|0x0D|1|Number of sectors per cluster.|
|14|0x0E|2|Number of reserved sectors. The boot record sectors are included in this value.|
|16|0x10|1|Number of File Allocation Tables (FAT's) on the storage media. Often this value is 2.|
|17|0x11|2|Number of root directory entries (must be set so that the root directory occupies entire sectors).|
|19|0x13|2|The total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume, and the actual count is stored in the Large Sector Count entry at 0x20.|
|21|0x15|1|This Byte indicates the [media descriptor type](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#BPB20_OFS_0Ah).|
|22|0x16|2|Number of sectors per FAT. FAT12/FAT16 only.|
|24|0x18|2|Number of sectors per track.|
|26|0x1A|2|Number of heads or sides on the storage media.|
|28|0x1C|4|Number of hidden sectors. (i.e. the LBA of the beginning of the partition.)|
|32|0x20|4|Large sector count. This field is set if there are more than 65535 sectors in the volume, resulting in a value which does not fit in the _Number of Sectors_ entry at 0x13.|
For us to create our own fake BPB, we'll need to fill at MOST 33 bytes of data without the first three ones, because those are reserved for `EB 3C 90` or `JMP SHORT 3c NOP`. `3c` is arbitrary and can be our own code, in our case it will be the `start` label.

### The parameter block in 8086 Assembly.
```
_start:
	; when we said that JMP SHORT 3c NOP, that 3c is arbitrary, this is it.
	jmp short start
	nop

times 33 db 0
; the actual BPB. 

start:
	jmp 0x7c0:step2

step2:
	; old code...
```

# 5. Writting our bootloader into a USB.
```
sudo dd if=./boot.bin of=/dev/sdb # your USB goes in of.
```
# 6. The Interrupt Vector Table
- Interrupts are similar to subroutines, but we don't need to know the memory address to invoke them.
- Interrupts are called via interrupt numbers rather than memory addresses.
- Interrupts can be setup by the programmer. We could point `0x32` to a piece of our code and if someone else calls `0x32`, they will invoke our code.
## What happens when we invoke an interrupt?
- The processor is interrupted.
- The old state is saved in the stack.
- The interrupt is executed.
## What is the interrupt vector table?
- Table describing all 256 interrupt handlers.
- Interrupts contain 4 bytes (offset:segment).
- Interrupts are in numerical order in the table.

Interrupts, being 4 bytes each, are in the following order:
- Interrupt 0: 0x00
- Interrupt 1: 0x04
- Interrupt 2: 0x08
- Interrupt 3: 0x0C
...and so on and so forth.

For example, take this IVT:

| Offset  | Segment | Offset  | Segment | Offset  | Segment | Offset  | Segment |
| ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- |
| 0x00    | 0x7C0   | 0x8D00  | 0x00    | 0x00    | 0x8D0   | 0x7C00  | 0x587   |
| 2 bytes | 2 bytes | 2 bytes | 2 bytes | 2 bytes | 2 bytes | 2 bytes | 2 bytes |
Each interrupt is four bytes, and the formula to get the physical address (segment * 16 + offset) still applies here.

The processor can also call for interrupts. For example, the Intel processor will call interrupt 0 whenever we divide anything by zero, because the interrupt zero is the interrupt for handling division by zero.

# 7. Implementing our own interrupts in realmode.
```
handle_zero:
	; literally just print A when division by zero occurs.
	mov ah, 0eh
	mov al, 'A'
	mov bx, 0x00
	int 0x10
	iret

step2:
	; old code...
	; interrupt code now!
	mov word[ss:0x00], handle_zero
	mov word[ss:0x02], 0x7c0 ; its not 0x04 because 0x7c0 is two bytes, finishing at 0x03.
	; this will put our handle_zero subroutine at interrupt 0.
	mov word[ss:0x04], handle_two  ; now we do use 0x04. 
	mov word[ss:0x06], 0x7c0 ; and 0x7c0 finishes at 0x07.
	; the same, but with interrupt 2.
	; if you're thinking
```
## iret
- When writting interrupts, we don't use the `ret` instruction. We use `iret`, aka ***interrupt return***. 
## mov word[ss:0x00], handle_zero
- We're moving our subroutine into 0x00, or the interrupt zero, filling up 0x00 and 0x01.
## mov word[ss:0x02], 0x7c0 
- Now we're moving 0x7c0 to 0x02, filling up 0x02 and 0x03, finishing our four byte interrupt.

### Exceptions ([read more here](https://wiki.osdev.org/Exceptions))
- When dividing by zero, interrupt 0 is called by an exception. So interrupt 0 is an exception.

# 8. Disk access and how it works.
## Files do not actually exist.
- Filesystems are kernel implemented. They are not the responsibility of the disk to handle.
- Implementing a filesystem requires the kernel programmer to create a filesystem driver for the target filesystem.
# Data is typically written in sectors.
- Each sector is typically 512 bytes per sector in modern disks.
- CDs, for example, blocksize might be bigger.
- Reading the sector of a disk will return 512 bytes of data for the chosen sector.
## CHS (Cylinder Head Sector)
- Secotors are read and written by specifying a "head", "track" and "sector".
- This is an old fashioned and more complex way of reading from a disk drive.
## LBA (Logical Block Address)
- This is the modern way of reading frmo a hard disk, rather than specify "head", "track" and "sector", we just specify a number that starts from zero.
- LBA allows us to read from the disk as if we are reading blocks from a very large file.
- LBA 0 and LBA 1 are the first and second sectors of a disk, and together they're 1024 bytes.
## Calculating LBA
- If we want to read the byte at position 58376 on the disk, how do we do it?
- LBA: `58376 / 512 = 114.015625`
	- Now we have the sector, but we need to also calculate the offset, as we don't know if it will divide correctly.
	- We load 512 bytes into memory, now we need to calculate the offset.
- LBA Offset: `58,376 % 512 = 8`
- Let's confirm we're right.
- `114 * 512 = 58,368`
- `58,368 + 8 = 58,376`
## BIOS Disk routines.
- In 16 bit real mode, the BIOS provides interrupt `13h` for disk related operations.
- In 32 bit protected mode, we have to write our own disk drive, which is a bit more complicated.
# 9. Reading from the hard disk.
- So now we'll read from the hard disk. Not an actual hard disk, but a sector appended to our boot.bin image with dd. Nevertheless, the operation in a real drive is the same.
## The code.
```
; new code. this goes inside the step2 label.
step2:
	...
        ; new code
        mov ah, 2 ; read sector command
        mov al, 1 ; one sector to be read
        mov ch, 0 ; cylinder low eight bits
        mov cl, 2 ; sector number
        mov dh, 0 ; head number
        mov bx, buffer
        ; we do not need to set dl / drive number, as this is done
        ; automatically by the bios.
        int 0x13
        jc error

        mov si, buffer
        call print
        jmp $

; new!
error:
        mov si, error_message
        call print
        jmp $

print:
        mov bx, 0
.loop:
        lodsb
        cmp al, 0
        je .done
        call print_char
        jmp .loop
.done:
        ret

print_char:
        mov ah, 0eh
        int 0x10
        ret

; new!
error_message: db 'Failed to load sector.', 0

; old code. nothing new.
times 510-($-$$) db 0
dw 0xAA55

; new!
buffer:
```

## mov ah, 2 ; read sector command
- We move 2 to `ah`, 2 being is the read sector command. [for more info, read this!](https://www.ctyme.com/intr/rb-0607.htm)
## mov al, 1 
- We move 1 to `al`, beacuse we'll be reading one sector (as per de documentation linked above).
## mov ch, 0
- This is the low eight bits of the cylinder number. We set it to zero.
## mov cl, 2 
- The sector to be read, from 1 to 63. In our case, since the sector 1 is the bootloader itself, we'll read sector 2.
## mov dh, 0 
- This is the head number. 
## (implicit) mov dl, DRIVENUMBER
- Although we don't set this, the BIOS does. It's for the drive number.
## mov bx, buffer
- We move our buffer (outside the first sector) into `bx`, where the sector 2 will be loaded.
## int 0x13
- We run the load sector in RAM interrupt.
## jc error
- jc (jump carry) is a conditional jump where it will jump to `error` if the `cf` (carry flag) is set. As per the documentation, the `cf` register will be set if any errors are found during the load sector to RAM interrupt.
## mov si, buffer
- We load the buffer into the `si` register.
## error: mov si, error_message
- We load the `error_message` constant into the `si` register to print it out.
## buffer:
- This is a label created outside of the bootloader 512 bytes sector. It's used as a space we can use to print out the message found in the second sector.
![[Pasted image 20250501004456.png]]

