# 1. What is protected mode?
- Processor state in x86 architectures.
- Can provide memory and hardware protection.
- It has different memory schemes.
- It has 4GB of addressable memory.
## Protected mode security rings
- Protected mode allows us to protect memory from being accessed.
- It also prevents user programs to talk directly with the hardware.
![[Pasted image 20250501004752.png]]
## Different memory schemes.
- Selectors (CS, DS, ES, SS), etc.
- Paging (Remapping Memory Addresses).

### Selector memory scheme
- Our segmentation registers become selector registers.
- Selectors point to data structures that describe memory ranges and the permissions (ring level) required to access a given range.
![[Pasted image 20250501005054.png]]

### Paging Memory Scheme
- Memory is virtual and what you address can point to somehwere entirely different in memory.
- Memory protection is easier to control.
- Paging is the most popular choice for emmory schemes with kernels/operating systems.
![[Pasted image 20250501005143.png]]
### 4GB of addressable memory.
- We gain access to 32 bit instructions and can easily work with 32-bit registers.
- We can address up to 4GB ofg memory at any time and we are no longer limited to the 1MB of memory provided by real mode.
# 2. Switching to protected mode.
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

- `gdt_descriptor`: This is the GDT descriptor label.
	- `dw gdt_end - gdt_start-1`: This is the size of the GDT in bytes minus 1, as per CPU requirements.
        - `dd gdt_start`: This is the address of the GDT in memory.
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

![[Sin título 4.jpg]]
# 3. Setting up the A20 Line.
- Setting up the A20 line is necessary, as it is ["**the physical representation of the 21st bit (number 20, counting from 0) of any memory access**"](https://wiki.osdev.org/A20_Line). Without it, we won't be able to access to the 21st bit of any memory address.

```
in al, 0x92
or al, 2
out 0x92, al
```
- in al, 0x92: This instruction reads the current state of the A20 line from the SCPA port and stores it into the `al` register.
	- `in`: This reads data from the I/O port.
	- `al`: This is the destination register that will read the data from the I/O port.
	- `0x92`: is the address of the I/O port that controls the A20 line. This port is also known as the System Control Port A or SCPA.
- or al, 2: The `or` operation sets the A20 line enable bit (second bit) in the `al` register to `1`, thus enabling the A20 line. The other bits in the `al` register remain untouched.
	- `or`: Simple bitwise OR operation.
	- `al`: Register that holds the value of the current status of the A20 line.
	- `2`: Decimal value for 00000010, meaning that it has the second bit set, that bit being the enable A20 line.
- out 0x92, al: This lines writes the changes done to the status of the SCPA port, enabling the A20 line.
	- `out`: It's an input instruction that writes data to an I/O port.
	- `0x92`: It's the destination address, meaning that we'll write data to the SCPA port.
	- `al`: It's the OR'd register that contains the previous status of the SCPA port, but now with the 2nd bit set to 1, enabling the A20 line.
# 4. Setting up a cross-compiler.
Just do it you pussy. [Read this.](https://wiki.osdev.org/GCC_Cross-Compiler#Installing_Dependencies).

# 5. Loading our 32 bit kernel into memory and working with debugging symbols.
This shit got very complex very quick. Let's see it step by step.
## 5.1. Creating an initial kernel.
### 5.1.1. Makefile
To create our initial kernel, we'll take the `laod32` code and put it into a different assembly file called `kernel.asm`. Now, we need to setup a real Makefile for it to work and for it to have debugging symbols, so it's easier for us to debug it in GDB.
```make
FILES = ./build/kernel.asm.o

all: ./bin/kernel.bin ./bin/boot.bin
	# remove the older os.bin file if it exists.
        rm -rf ./bin/os.bin 
        # write the boot.bin into os.bin in pure raw binary.
        dd if=./bin/boot.bin >> ./bin/os.bin
        # write the kernel.bin into os.bin in raw binary, at sector 2.
        dd if=./bin/kernel.bin >> ./bin/os.bin
        # fill up the remaining space. count could be just one, but we
	# use 100 for testing the LBA driver.
        dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/kernel.bin: $(FILES)
	# first we link our kernel...
        i686-elf-ld -g -relocatable $(FILES) -o build/kernelfull.o
        # now we build with our linker.ld script and the kernel object we generated before.
        # the -ffreestanding, -O0 and -nostdlib are required.
        i686-elf-gcc -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	# just building the bootloader as we've done before...
        nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	# now we're building the kernel in ELF format.
        nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

clean:
        rm -rf ./bin/boot.bin
        rm -rf ./bin/os.bin
        rm -rf ./build/kernel.asm.o
```
### 5.1.2. linker.ld
We need to create a linker file to link out bootloader with our kernel and the kernel with the kernel object:
```
# we define the entrypoint for our program. in our case, it'll be the _start symbol (see kernel.asm)
ENTRY(_start)
# now we tell the linker how to output our file, i.e., a binary format with no headers at all.
OUTPUT_FORMAT(binary)
# we start defining the sections of our program.
SECTIONS
{
	# we tell the linker to load our file at RAM address 1M or 0x100000.
        . = 1M;
        # this section defines a .text section, which contains the code of our program.
        .text :
        {
                *(.text)
        }
        # this section defines .rodata, which contains read only data.
        .rodata :
        {
                *(.rotdata)
        }
        # this section defines .data, which contains initialized data, like global variables.
        .data :
        {
                *(.data)
        }
        # this section defines .bss, which contains non initialized data, like non initialized
        # global variables.
        .bss :
        {
                *(COMMON)
                *(.bss)
        }
}
```
### 5.1.3. kernel.asm
The kernel.asm file is the same `load32` routine we had previously. But in this case, we renamed it to \_start.
```
[BITS 32]
global _start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
        ; beginning of register setup
        mov ax, DATA_SEG
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov ebp, 0x00200000
        mov esp, ebp
        ; end of register setup
        ; enabling the A20 line.
        in al, 0x92
        or al, 2
        out 0x92, al
        ; end of enabling the A20 line.
        jmp $
```
- `[BITS 32]`: same as `[BITS 16]`, but now we're on 32 bit protected mode.
- `global _start`: we make the `_start` symbol global, so our compiler and linker can see it.
Now onto the complex part of the chapter...
### 5.1.4. The LBA driver.
The LBA driver is the part of the bootloader that allows us to read from the disk and put stuff into memory. It's an essential part in executing our kernel code.
#### 5.1.4.1. .load_protected
We still need the `load_protect` function, as we'll be working in protected mode rather than in realmode.
```
; we still have the load32 function in our bootloader.
.load_protected:
        cli
        lgdt [gdt_descriptor]
        mov eax, cr0
        or eax, 0x1
        mov cr0, eax
        jmp CODE_SEG:load32
```
#### 5.1.4.2. New `load32`
```
[BITS 32]
load32:
        mov eax, 1
        mov ecx, 100
        mov edi, 0x0100000
        call ata_lba_read
        jmp CODE_SEG:0x0100000
```
Now, instead of `jmp $` to just be able to sit in 32 bit protected mode, we need to write a driver that allows us to read sectors and write them into memory. Get ready.

There are also a few changes done. For example, now we aren't initializing the data segments of the RAM, or enabling the A20 line. This is because now that's something the kernel will do.

- `mov eax, 1`: In here we're setting up the amount of sectors to be read during the LBA read routine. If we were to read from 0, we'd be reading the bootloader.
- `mov ecx, 100`: This is the amount of sectors to be read.
- `mov edi, 0x0100000`: This is the memory address where our code will jump to later on, which will be the kernel.
- `call ata_lba_read`: This calls the `ata_lba_read` routine.
- `jmp CODE_SEG:0x0100000`: This will execute our kernel.

#### 5.1.4.3. ata_lba_read
This is the driver. We'll be looking at it in chunks; we won't need it in kernel space, but I'd like to understand what's going on here.

To actually understand what its being done, we need to know how to calculate the LBA address. Check this formula:

##### [Info on ATA and other I/O ports here.](https://wiki.osdev.org/I/O_Ports)

- `0x1F6`: This is the port address for the ATA drive's control register. Specifically, it's used to select the drive and set the LBA mode.
- `0x1F2`: This is the port address for the ATA drive's sector count register. It's used to specify the number of sectors to read or write.
- `0x1F3`: This is the port address for the ATA drive's LBA low register. It's used to send the lower 8 bits of the LBA address to the drive.
- `0x1F4`: This is the port address for the ATA drive's LBA mid register. It's used to send the middle 8 bits of the LBA address to the drive.
- `0x1F5`: This is the port address for the ATA drive's LBA high register. It's used to send the upper 8 bits of the LBA address to the drive.
- `0x1F7`: This is the port address for the ATA drive's command register. It's used to send commands to the drive, such as "read sector" or "write sector".
- `0x1F0`: This is the port address for the ATA drive's data register. It's used to read or write data from the drive.

>`LBA address = (ecx >> 24) + (ecx >> 16) + (ecx >> 8)`

In the assembly, `ecx` is right shifted three times and sent to the I/O bus

```
ata_lba_read:
        mov ebx, eax ; backup the lba
        ; send the highest 8 bits to the hard disk controller
        shr eax, 24
        or eax, 0xE0 ; selects the master drive
        mov dx, 0x1F6
        out dx, al
        ; finish sending the highest 8 bits of the lba
```

- `ata_lba_read:`: define the `ata_lba_read` routine label.
- `mov ebx, eax `: we backup the value of the LBA into the `ebx` register.
- `shr eax, 24`: following the LBA address formula, we're right shifting `eax`. Although the formula states `ecx`, it doesn't matter as long as we're working with LBA values. 
	- By right shifting eax (1) by 24, we're effectively doing `2^24`, which is equivalent to extracting the highest 8 bits of a 32 bit value.
	- Meaning that now, we have the highest 8 bits in our `eax` register. 
- `or eax, 0xE0 `: selects the master drive by ORing `eax`. if we wanted to select the slave disk, we'd use `0xF0`.
	- This value needs to be ORd and sent to the primary bus.
- `mov dx, 0x1F6`: This will set the `dx` register to `0x1F6`, which will allow us to talk with the `0x1F6` ATA port.
- `out dx, al`: Send the data.

Let's continue with the following section.
```
	; send the total sectors to read
        mov eax, ecx
        mov dx, 0x1F2
        out dx, al
        ; done sending the sectors to read
```
- `mov eax, ecx`: We set up the total amount of sectors to read, so later we'll send this data to the ATA interface.
- `mov dx, 0x1F2`: Now we set `dx` to `0x1F2` (LBA Sector Count)
- `out dx, al`: This sends the first 8 bits of the `eax` register to the `0x1F2` ATA I/O port. See above for more info.
```
        mov eax, ebx ; restoring the lba backup
        mov dx, 0x1F3
        out dx, al
        ; finished sending more bits of the LBA

```

- `mov eax, ebx`: we restore the LBA backup.
- `mov dx, 0x1F3`: we set the I/O interface to `0x1F3` (LBA Low Register)
- `out dx, al`: we send the lower 8 bits of the EAX register to the I/O interface.

```
        mov dx, 0x1F4
        mov eax, ebx ; restoring again the LBA backup
        shr eax, 8
        out dx, al
        ; finished sending even more bits of the LBA
```

- `mov dx, 0x1F4`: we set the I/O interface to `0x1F4` (LBA Mid Register)
- `mov eax, ebx`: we restore the LBA backup yet again
- `shr eax, 8`: we right-shift the value of `eax` by 8 (as per LBA Address formula above)
- `out dx, al`: we send the data to the `0x1F4` I/O interface.

```
        ; send uppter 16 bits
        mov dx, 0x1F5
        mov eax, ebx ; restoring the LBA backup
        shr eax, 16
        out dx, al
```

- `mov dx, 0x1F5`: we set the I/O interface to `0x1F5` (LBA High Register).
- `mov eax, ebx`: we restore the LBA backup.
- `shr eax, 16`: we right-shift the value of `eax` by 16 (as per LBA Address formula above)
- `out dx, al`: we send the data to the `0x1F5` I/O interface.

```
        ; finished sending upper 16 bits of the LBA
        mov dx, 0x1F7
        mov al, 0x20
        out dx, al
```

- `mov dx, 0x1F7`: finally, we set the I/O interface to `0x1F7` (LBA Command Register).
- `mov al, 0x20`: we set the LBA action to read.
- `out dx, al`: we finally send the last bits of data to the `0x1F7` interface.

```
.next_sector:
        push ecx
```
        
- `.next_sector:`: we define a label that will allow us to decrement the sector value to be read.
- `push ecx`: we push the value of `ecx` (100, the amount of sectors to be read) into the stack.

```
; checking if we need to read
.try_again:
        mov dx, 0x1F7
        in al, dx
        test al, 8
        jz .try_again
```

- `.try_again:`: this label will help us to check if we have to read from disk or not, as sometimes the disk may have a bit of a delay.
- `mov dx, 0x1F7`: again as before, we set the I/O interface to `0x1F7` (LBA Command Register).
- `in al, dx`: we set the action register to read.
- `test al, 8`: we check if the number `8` is in the `al` register. if so, continue; if not...
- `jz .try_again`: try again!

```
; we need to read 256 words at a time
        mov ecx, 256
        mov dx, 0x1F0
        rep insw
        pop ecx
        loop .next_sector
        ; end of reading sectors into memory
        ret
```

- `mov ecx, 256`: we set the amount of words to be read (translate to 512 bytes).
- `mov dx, 0x1F0`: we now set the interface to be read into the `0x1F0` (LBA Data Register)
- `rep insw`: ...
	- `rep`: repeats an instruction X amount of times defined in the `e[cx]` register.
	- `insw`: inputs word from I/O port specified in the `dx` register into memory pointer by the `e[di]` register.
		- in our case, `edi` is `0x0100000`. check [[#5.1.4.2. New `load32`]] for a refresher.
	- for more info, read the [[General Assembly Notes]] page.
- `pop ecx`: we now `pop` the value of `ecx` out of the stack.
- `loop .next_sector`: the `loop` instruction compares the value of `e[cx]` and checks if it's zero. If not, it decrements `ecx` by 1 and loops again.
- `ret`: when `ecx` equals zero, the function will return to `load32`.

## 6. Potential alignment issues.
When linking, our C code might be misaligned. For example:
- In a section (.text) we might have Assembly code. But, if we have C code, there's nothing that could assure us that the C code will be interpreted as code, and not as data or rodata.
- This might be an issue later on. So to fix it, we'll modify our linker script:

```
ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
        . = 1M;
        .text : ALIGN(4096)
        {
                *(.text)
        }

        .rodata : ALIGN(4096)
        {
                *(.rotdata)
        }

        .data : ALIGN(4096)
        {
                *(.data)
        }

        .bss : ALIGN(4096)
        {
                *(COMMON)
                *(.bss)
        }
        .asm : ALIGN(4096)
        {
                *(.asm)
        }
}
```
We've added the `ALIGN(4096)` sentence after the declaration of the sections and created the new `.asm` section, where we'll put our Assembly code in the future, but NOT the kernel! The kernel will still go in the `.text` section, as it'll be the first thing to be loaded, so we need to execute it as quickly as possible.

## 7. C Code in Protected Mode.
First of all, we need to make a few changes in our Makefile.
```
FILES = ./build/kernel.asm.o ./build/kernel.o # added kernel.o
INCLUDES= -I./src # added this include line
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-functions -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
# added aaaaaaaall of this GCC flags.
[SNIP]
./bin/kernel.bin: $(FILES)
	...
        i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o
	# we added the FLAGS to the linker script.
[SNIP]
./build/kernel.o: ./src/kernel.c
        i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o
        # we created this gcc command to build the kernel.c file.
```
## 7.1. GCC Flags
#### 7.1.1.1.  Debugging and optimization flags
- `-g`: Generates debugging information, such as symbol tables and line numbers, that can be used by debuggers like GDB.
- `-O0`: Disables all optimizations, which can make the code easier to debug but may result in slower performance.
#### 7.1.1.2. Alignment flags
- `-falign-jumps`: Aligns jump targets to a power-of-2 boundary, which can improve performance on some architectures.
- `-falign-functions`: Aligns function entries to a power-of-2 boundary, which can improve performance on some architectures.
- `-falign-labels`: Aligns labels to a power-of-2 boundary, which can improve performance on some architectures.
- `-falign-loops`: Aligns loops to a power-of-2 boundary, which can improve performance on some architectures.
#### 7.1.1.3. Code generation flags
- `-ffreestanding`: Tells GCC to generate code that doesn't rely on the standard library or startup code, which is useful for embedded systems or operating system development.
- `-fstrength-reduce`: Enables strength reduction, which is an optimization that replaces expensive operations with cheaper ones.
- `-fomit-frame-pointer`: Omits the frame pointer, which can reduce code size but may make debugging more difficult.
- `-finline-functions`: Inlines functions, which can improve performance by reducing function call overhead.
#### 7.1.1.4. Warning and error flags
- `-Wno-unused-functions`: Disables warnings about unused functions.
- `-Werror`: Treats all warnings as errors, which can help catch potential issues early.
- `-Wno-unused-label`: Disables warnings about unused labels.
- `-Wno-cpp`: Disables warnings about C++-specific issues, which is useful when compiling C code.
- `-Wno-unused-parameter`: Disables warnings about unused function parameters.
- `-Wall`: Enables all warnings, which can help catch potential issues early.
#### 7.1.1.5. Library and startup flags
- `-nostdlib`: Disables the standard library, which is useful for embedded systems or operating system development.
- `-nostartfiles`: Disables the startup files, which are used to initialize the program.
- `-nodefaultlibs`: Disables the default libraries, which are used to provide common functions like `printf`.
#### 7.1.1.6. Include flag
`-Iinc`: Adds the `inc` directory to the include path, which allows GCC to find header files in that directory.

## 7.2. Kernel.c and Kernel.h files
We created two files that will be our kernel C code and the header file. They look like this:
```
/* kernel.c file */
#include "kernel.h"

void kernel_mail()
{

}
```
And the header file
```
/* kernel.h file */
#ifndef KERNEL_H
#define KERNEL_H
#endif

void kernel_main();
```
## 7.3. Modifications to `kernel.asm`
We modified a couple things in our `kernel.asm` file:
```
# we added this extern directive to call our C code from thd
# assembly code
extern kernel_main


_start:
	[snip]
	# we've also added this call kernel_main, to actually call the C code.
	call kernel_main
```
## 8. Text mode explained
- Text mode allows us to write ASCII to video memory.
- It supports 16 unique colors.
- There's no need to set individual screen pixels for printing characters.
## 8.1. Text mode allows us to write ASCII to video memory.
- We write ASCII characters into memory starting at address `0xB8000` for color displays.
- Or for monochrome displays at `0xB0000`
- Each ASCII character written to memory has its pixel equivalent outputted to the monitor.
## 8.2. No need to set individual screen pixels.
- In text mode, we can directly output the ASCII value to the screen instead of writting pixel by pixel.
## 8.3. Each character takes up two bytes.
- Byte 0: ASCII character; 'A', for example,
- Byte 1: Color code; '0x00' for black, for example.
### 8.3.1. We want to set row 0 column 0 to black `a`.
```
# row 0 column 0 to black A
0xb8000 = 'A'
0xb8001 = '0x00'
# row 0 column 1 to black B
0xb8002 = 'B'
0xb8003 = '0x00'
```
# 9. Hello World, again!
Now that we know the VGA video memory address, we can start writting stuff to the terminal.

## 9.1. Simple video_mem write.
```
void kernel_mail()
{
	char* video_mem = (char*)(0xB8000);
	video_mem[0] = 'A';
	video_mem[1] = 1;
}
```
- `char* video_mem = (char*)(0xB8000);`: we create a pointer to the `0xB8000` memory address.
- `video_mem[0] = 'A';` we write A to the first byte and...
- `video_mem[1] = 1;` one to the second byte, as we want the color to be black.
For demonstrative purposes, we'll use values 'Z' and 15, meaning a letter Z in white color.
![[Pasted image 20250506124938.png]]

Let's continue. Now, instead of writting byte by byte, we'll simply write hexadecimal values.

## 9.2. Writting bytes to video_mem
```
#include<stdint.h>

void kernel_main()
{
	uint16_t video_mem = (uint16_t)(0xB8000);
	video_mem[0] = 0x5AF;
}
```

- `uint16_t video_mem = (uint16_t)(0xB8000);`: using a `uint16_t` type instead of a `char` will allow us to write two bytes directly into memory instead of just one.
- `video_mem[0] = 0x5AF;`: here we assign the values in hex, similar to what we did before.

But! What the hell is that?!
![[Pasted image 20250506125226.png]]
While our code is correct, our bytes are not! We're working within little endian architectures, meaning that the least significant bytes go first. So it should be `0xF5A`! 
## 9.2.1. Fixing endiannes!
```
#include<stdint.h>
// this include will be put at the kernel.h header file.

void kernel_main()
{
	uint16_t video_mem = (uint16_t)(0xB8000);
	video_mem[0] = 0xF5A;
}
```
Here we just reverse the endianness of the `0xF5A` bytes.

![[Pasted image 20250506125604.png]]
Now it works! But writting to the VGA memory using hexadecimal characters is kind of a pain in the ass. So, we'll write a routine that will help us do that for us.
## 9.3. Making our program write bytes for us!
```
void terminal_make_char(char c, char color)
{
	return (color << 8) | c;
}
void kernel_main()
{
	uint16_t* video_mem = (uint16_t*)(0xB8000);
	video_mem[0] = terminal_print_char('Z', 15);
}
```

- `return (color << 8) | c;`: this simple formula will align the characters and bytes for us. For a more detailed explaination, consult [[Text mode notes]].
	
![[Pasted image 20250506125923.png]]
Yipe!!! Now we're playing! But it looks quite ugly with all the BIOS information and stuff. We'll write a terminal initialization function that will clear the screen for us.
## 9.4. Clearing the screen.
```
#define VGA_WIDTH 80
#define VGA_HEIGHT 20
// both of this define values will be put in the kernel.h file.

uint16_t* video_mem = 0;
void terminal_print_char(char c, char color)
{
	return (color << 8) | c
}

void terminal_initialization() {
	video_mem = (uint16_t)(0xB8000);
	for (y=0; y < VGA_HEIGHT; y++)
	{
		for (x=0; x < VGA_WIDTH; x++)
		{
			video_mem[(y*VGA_WIDTH) + x] = terminal_print_char(' ', 0);
		}
	}
}
void kernel_main()
{
	terminal_initialization();
}
```

- `video_mem = (uint16_t)(0xB8000);`: as before, we assign `video_mem` as a `uint16_t` to be able to access the two bytes we need.
- `for (y=0; y < VGA_HEIGHT; y++)`: we start to go along the 80 columns of our VGA screen.
- `for (x=0; x < VGA_WIDTH; x++)`: and now we begin to go along the 20 rows of our VGA screen.
- `video_mem[(y*VGA_WIDTH) + x] = terminal_print_char(' ', 0);`: this formula helps us calculate the exact index in a one-dimensional matrix. for a more detailed explaination, consult [[Text mode notes]].

![[Pasted image 20250506130649.png]]
Yay! The screen has been cleared! :) The expression ((y+VGA_WIDTH)+x) is explained in [[Text mode notes]].
## 9.5. Creating a `print` function.
Now that we have all of this functions created, we can create a new one, a `print` function. But we'll have to write a bit more code before actually having a `print` function.

### 9.5.1. `strlen` implementation.
We'll need to implement a simple `strlen` function. For that, we can do the following:
```
#include<stddef.h>

size_t strlen(const char* str) 
{
	size_t len = 0;
	while(str[len])
	{
		len++;
	}
	return len;
}
```
- `size_t len = 0;`: we initially assign `len` to 0.
- `while(str[len])`: this will iterate over every char in our `str`. and if `str` is empty, it will return `len = 0`, which would be correct.
- `len++;`: if `str[len]` isn't zero, increment `len`.
- `return len;` after finishing reading the `str`, we return `len`.
### 9.5.2. `terminal_putchar`
Now, we want to create a function that will directly put the bytes of data that we want into memory. This way, we won't have to refer directly to `video_mem`.
```
void terminal_putchar(int x, int y, char c, char color)
{
	video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, color);
}
```
- `video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, color);`: this is just a combination of what we've seen before.
### 9.5.3. `terminal_writechar`
This function will call `terminal_putchar` and put the bytes into the correct VGA memory address in terms of columns and rows, as to not overlap characters.
```
int terminal_row = 0;
int terminal_col = 0;

void terminal_writechar(char c, char color)
{
	terminal_putchar(terminal_col, terminal_row, c, color);
	terminal_col += 1;
	if (terminal_col >= VGA_WIDTH)
	{
		terminal_row += 1;
		terminal_col = 0;
	}
}
```

- `terminal_putchar(terminal_col, terminal_row, c, color);`: now we call the previously defined function to add a character as `0,0`.
- `terminal_col += 1;`: after writting our character, we increment the `terminal_col`, as to not overlap our characters.
- `if (terminal_col >= VGA_WIDTH)`: now we check if we have to introduce a newline. if `terminal_col` is equal or higher than 80, we...
- `terminal_row += 1;`: go to a new row, and...
- `terminal_col = 0;`: reset the columns.

Now, whenever we want to print a character to the screen, we'll call `terminal_writechar`. But we need to keep abstracting it a bit more...
### 9.5.4. `print`
Now we'll write the print function! There are two implementations: the one I made, and the one the instructor made. I'll show both.
```
/* i like this implementation because i did it :) */
void terminal_writestring(const char* str, char color)
{
	size_t pos = 0;
	while(str[pos])
	{
		terminal_writechar((char) (str[pos]), color);
		pos++;
	}
}
```

- `size_t pos = 0;`: we create `pos`, which will we used to iterate over our string.
- `while(str[pos])`: here we iterate over `str[pos]`, and whenever `str` becomes \x00 (null terminator), it will let go.
- `terminal_writechar((char) (c[pos]), color);`: we cast and print our `str` as a single byte.
- `pos++;`: finally we increment `pos`.

```
/* this is the instructor's implementation. i think it's a bit cleaner. */
void print(const char* str)
{
	size_t len = strlen(str);
	for (int i = 0; i < len; i++)
	{
		terminal_writechar(str[i], 15);
	}
}
```

- `size_t len = strlen(str);`: we assign `len` to the return value of `strlen(str)`.
- `for (int i = 0; i < len; i++)`: now we iterate over `len`.
- `terminal_writechar(str[i], 15);`: we write our char.

Now our print function is done. But... what about newlines???
![[Pasted image 20250507104600.png]]
Aw fuck! We need to implement newlines.
### 9.5.5. Implementing newlines.
This is a very easy task. We just need to put an `if` statement at out `terminal_writechar` function to check if `c` is `\n` and what to do then.
```
void terminal_writechar(char c, char color)
{
        if (c == '\n')
        {
                terminal_row += 1;
                terminal_col = 0;
                return;
        }
        terminal_putchar(terminal_col, terminal_row,  c, color);
        terminal_col +=1;
        if (terminal_col >= VGA_WIDTH)
        {
                terminal_col = 0;
                terminal_row += 1;
        }
}
```

- `if (c == '\n')`: we check if `c` is a newline character (\n). if so...
- `terminal_row += 1;`: we increment our row number and...
- `terminal_col = 0;`: reset our columns!
- `return;`: after that, we return to the calling function, as passing to `terminal_putchar` would put a new newline.
Lets check it out!
![[Pasted image 20250507104948.png]]
*Note: I have a small issue. There's a new character at the beginning of `hello test!`, a blank space. I don't yet know why that is happening.*

# 10. Interrupt Descriptor Table Explained
In protected mode, the interrupt descriptor table is the equivalent to the interrupt vector table.

- It describes how interrupts are called in protected mode.
	- Similarly to the interrupt vector table, the interrupt descriptor table describes how interrupts are setup in the CPU so that if someone causes an int 5 it will invoke the code for interrupt 5 as decsribed by the interrupt descriptor table.
- It can be mapped anwyhere in memory.
- It's different from the Interrupt Vector Table.


| Name     | Bit   | Known As                   | Description                                                               |
| -------- | ----- | -------------------------- | ------------------------------------------------------------------------- |
| Offset   | 46-63 | Offset 16-31               | The higher part of the offset to execute.                                 |
| P        | 47    | Present                    | This should be set to zero for unused interrupts.                         |
| DPL      | 45-46 | Descriptor Privilege Level | The ring level the processor requires to call this interrupt.             |
| S        | 44    | Storage Segment            | Should be set to zero for trap gates.                                     |
| Type     | 40-43 | Gate Type                  | THe type of gate this interrupt is treated as.                            |
| 0        | 32-39 | Unused 0-7                 | Unused bits in this structure.                                            |
| Selector | 16-31 | Select 0-15                | The selector this interrupt is bounded to, i.e. the kernel code selector. |
| Offset   | 0-15  | Offset 0-15                | The lower part of the offset to execute.                                  |
## 10.1 Example implementation
```
struct idt_desc
{
	uint16_t offset_1; // offset bits 0..15
	uint16_t selector; // a code segment selector in GDT or LDT
	uint8_t zero;      // unused, set to 0
	uint8_t type_attr; // type and attributes
	uint16_t offset_2; // offset bits 16..31

} __attribute__((packed)); // we use this so the compiler doesn't rearrange our bits.
```
## 10.2 Gate (interrupt) types
Each entry on the Interrupt Descriptor Table is called a gate. There are several types of gates:


| Name                        | Value         | Description                                                                                                                                                  |
| --------------------------- | ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 80386 32 bit Task Gate      | `0x05/0b0101` | Task gates reference TSS descriptors and can assist in multitasking when exceptions occur.                                                                   |
| 80386 16-bit Interrupt Gate | `0x06/0b0110` | Interrupt gates are to be used for interrupts that we want to invoke ourselves in our code.                                                                  |
| 80386 16-bit Trap Gate      | `0x07/0b0111` | Trap gates are like interrupt gates however they are used for exceptions. They also disable interrupts on entry and re-enable them on an "iret" instruction  |
| 80386 32-bit Interrupt Gate | `0x0E/0b1110` | Interrupts gates are to be used for interrupts that we want to invoke ourselves in our code.                                                                 |
| 80386 32-bit Trap Gate      | `0x0F/0b1111` | Trap gates are like interrupt gates however they are used for exceptions. They also disable interrupts on entry and re-enable them on an "iret" instruction. |
## 10.3 Interrupt Descriptor Array.
Interrupt descriptors are stored in an array with index 0 defining interrupt zero `int 0`. Index 1 defines interrupt 1, `int 1` and so on and so forth.
![[Pasted image 20250510221232.png]]

## 10.4 The IDTR: Interrupt Descriptor Table Register
The IDTR is a structure that points to the Interrupt Descriptor Table. It's very similar to the Global Descriptor Table's `gdt_descriptor` label that we created some time ago.

| Name  | Bit   | Description                                             |
| ----- | ----- | ------------------------------------------------------- |
| Limit | 0-15  | The length of the interrupt descriptor table minus one. |
| Base  | 16-47 | The address of the interrupt descriptor table.          |

## 10.5 IDTR Example implementation
```
struct idt_desc
{
	uint16_t limit;    // our 16 bit base.
	uint32_t base;     // our 32 bit address for the interrupt descriptor table.
} __attribute__((packed)); // we use this so the compiler doesn't rearrange our bits.
```
## 10.6 Loading the IDT
```
idt_load:
	push ebp
	mov ebp, esp
	mov ebx, [ebp+8]
	lidt [ebx]
	pop ebp
	ret
```
x86 INT instruction

# 11. Implementing the IDT in our code.
- explain memset.c and memset.h
- explain idt.c, idt.h and idt.asm

## 11.1 memory.c and memory.h
Before actually implementing our IDT, we need a way to zero out the descriptor.

```
// memory.h
#ifndef MEMORY_H
#define MEMORY_H
#include<stddef.h>

void memset(void* ptr, int c, size_t size);

#endif
```
- header guard.
	- `#ifndef MEMORY_H`: the preprocessor checks if the `MEMORY_H` constant is defined. if it is, it skips the header file. but if not...
	- `#define MEMORY_H`: we define `MEMORY_H` and read the rest of the header file.
	- header guard: header guard is what is done in this code by using the `ifndef` and `endif` macros. it makes sure that the header file is included just one time in our code; if, for example, `memory.h` was used in other place apart from `memory.c` and there was no header guard, the preprocessor would load the header the amount of times it is used in the code. the header guard prevents that and only allows it to be loaded once.
- `#include<stddef.h>`: standard def, for `size_t` and other types used in `memory.c`.
- `void memset(void* ptr, int c, size_t size);`: we define the prototype for our `memset` function.
- `#endif`: we end our header file.

```
// memory.c
#include "memory.h"

void* memset(void* ptr, int c, size_t size)
{
        char* c_ptr = (char*) ptr;
        for (int i = 0; i < size; i++)
        {
                c_ptr[i] = (char) c;
        }
        return ptr;
}
```
- `#include "memory.h"`: we include our header file.
- `void* memset(void* ptr, int c, size_t size)`: we start to work on the `memset` function that will take a void pointer to the descriptor table, what will be put there in integer form and the size of the descriptor.
- `char* c_ptr = (char*) ptr;`: we create a pointer to the given pointer to be able to write to it. this is because we received a void pointer, and to be able to write to such a pointer, we need to define its type. in this case, we're casting it into our `char* c_ptr` pointer.
- `for (int i = 0; i < size; i++)`: we initialize a `for` loop that will set the memory to whatever we passed as `c`.
- `c_ptr[i] = (char) c;`: we cast the `c` argument and put it in the `i` position in memory of `c_ptr`, which points to `ptr`.
- `return ptr;`: at the end, we return the `ptr` passed and set to the given `c`.

## 11.2 config.h
Before explaining our IDT related code, we need to talk about the newly created `config.h` file.
```
// config.h
#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

#define PEACHEOS_TOTAL_INTERRUPTS 512

#endif
```
- `#ifndef CONFIG_H`: header guard, standard stuff.
- `#define CONFIG_H`: header guard, standard stuff.
- `#define KERNEL_CODE_SELECTOR 0x08`: here we set `KERNEL_CODE_SELECTOR` to `0x08`, which is our `CODE_SEG` in the GDT.
- `#define KERNEL_DATA_SELECTOR 0x10`: here we set `KERNEL_DATA_SELECTOR` to `0x10`, which is our `DATA_SEG` in the GDT.
- `#define PEACHEOS_TOTAL_INTERRUPTS 512`: here we set the amount of interrupts that we'll have.
- `#endif`: we end our header file.

## 11.3 idt.c and idt.h
Now, we'll write the functions that create the prototype structures for our IDT and the C code that actually uses them.
```
// idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>
struct idt_desc
{
        uint16_t offset_1; // offset bits 0 - 15
        uint16_t selector; // selector in our GDT
        uint8_t zero;      // does nothing; bits are reserved.
        uint8_t type_attr; // descriptor type and attributes.
        uint16_t offset_2; // offset bits 16-31

} __attribute__((packed));

struct idtr_desc
{
        uint16_t limit;    // size of the descriptor table - 1
        uint32_t base;     // base address of the start of the interrupt table.
} __attribute__((packed));

#endif
```

- `#ifndef IDT_H`: we define our header guard
- `#define IDT_H`: we set define the `IDT_H` constant, so the header isn't loaded two times.
- `#include <stdint.h>`: we include `stdint` for our types.
- `struct idt_desc`: we create our Interrupt Descriptor Table.
- `        uint16_t offset_1; // offset bits 0 - 15`: 
- `        uint16_t selector; // selector in our GDT` 
- `        uint8_t zero;      // does nothing; bits are reserved.`
- `        uint8_t type_attr; // descriptor type and attributes.` 
- `        uint16_t offset_2; // offset bits 16-31` 
- `struct idtr_desc`: now we create the Interrupt Descriptor Table Register.
- `        uint16_t limit;    // size of the descriptor table - 1`: 
- `        uint32_t base;     // base address of the start of the interrupt table.`: 
- `#endif`: end the header file.

```
// idt.c
#include "idt.h"
#include "config.h"
#include "memory.h"
#include "kernel.h"

extern void idt_load(struct idtr_desc* ptr);

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

void idt_zero()
{
        print("divide by zero error\n");
}

void idt_set(int interrupt_no, void* addr)
{
        struct idt_desc* desc = &idt_descriptors[interrupt_no];
        desc->offset_1 = (uint32_t) address & 0x0000ffff;
        desc->selector = KERNEL_CODE_SELECTOR;
        desc->zero = 0x00;
        desc->type_attr = 0xEE;
        desc->offset_2 = (uint32_t) addr >> 16;
}

void idt_init()
{
        memset(idt_descriptors, 0, sizeof(idt_descriptors));
        idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
        idtr_descriptor.base = idt_descriptors;
        idt_set(0, idt_zero);

        // load interrupt descriptor table
        idt_load(idtr_descriptor);
}
```
- `#include "idt.h"`: we include our header file.
- `#include "config.h"`: we include our config file.
- `#include "memory.h"`: we include our `memory.h` header file.
- `extern void idt_load(struct idtr_desc* ptr);`: now we add the externally created `idt_load` function, more on that later.
- `struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];`: now we create `idt_descriptors` of `idt_desc` type with [PEACHOS_TOTAL_INTERRUPTS] amount of interrupts. this is our IDT.
- `struct idtr_desc idtr_descriptor;`: now we create `idtr_descriptor` of type `idtr_desc`, which will function as our IDTR.
- `void idt_zero()`: here we define the code that will be our interrupt 0.
- `print("divide by zero error\n");`: since Intel uses `0` for the divide by zero trap, we set it here.
- `void idt_set(int interrupt_no, void* address)`: this function will help us define our interrupts.
- `struct idt_desc* desc = &idt_descriptors[interrupt_no];`: we create a pointer structure to the memory address of `idt_descriptors` at the `interrupt_no` place.
- `desc->offset_1 = (uint32_t) address & 0x0000ffff;`: here we set our `offset_1` to the lower port of `addr`.
- `desc->selector = KERNEL_CODE_SELECTOR;`: here we set out selector, which is `0x08`.
- `desc->zero = 0x00;`: here we set the bits that need to be zero to zero.
- `desc->type_attr = 0xEE;`: here we define our attributes. in this case, `0xEE` is `11101110`. read [[Interrupt Descriptor Table Related Notes]] for more info on this.
- `desc->offset_2 = (uint32_t) address >> 16;`: here we set the offset to the higher part of the of `addr`.
- `void idt_init()`: here we start our `idt_init` function.
- `memset(idt_descriptors, 0, sizeof(idt_descriptors));`: here we zero out the IDT. even though it should be zero, we need to make sure that it is, indeed, zero.
- `idtr_descriptor.limit = sizeof(idt_descriptors) - 1;`: here we set the IDTR limit minus one. more on [[Interrupt Descriptor Table Related Notes]]
- `idtr_descriptor.base = (uint32_t) idt_descriptors;`: and here we set its base which is casted to `uint32_t`.
- `idt_set(0, idt_zero);`: now we setup our divide by zero interrupt.
- `idt_load(&idtr_descriptor)`: and here we call our `idt_load` function, which we'll see in a couple seconds.
## 11.4. idt.asm
Now, we'll create an Assembly routine that loads the IDT into the CPU. We cannot do this in C, so we **need** to make it in Assembly.
```
; idt.asm
section .asm

global idt_load
idt_load:
	push ebp
	mov ebp, esp
	mov ebx, [ebp+8]
	lidt [ebx]
	pop ebp
	ret
```

- `section .asm`: we need to add this Assembly to our previously defined `.asm` section. see the linker.ld file.
- `global idt_load`: here we make our `idt_load` function global. this will allow our C code to call it.
- `idt_load:`: here we start our `idt_load` function.
- `push ebp`: standard stuff: we push the base pointer to the stack to have it at hand.
- `mov ebp, esp`: here we move the base pointer to the stack pointer.
- `mov ebx, [ebp+8]`: here magic happens. `[ebp+8]` is a given argument. this means that `ebx` will hold our `idtr_descriptor` structure defined in our C code.
- `lidt [ebx]`: here we call `lidt` and load the Interrupt Descriptor Table.
- `pop ebp`: we pop `ebp`.
- `ret`: and we return.

now our IDT is loaded :) although I had some doubts on why we had to call `ebp+8`, so I asked Ollama:

```
In x86 assembly language, when you use the `push ebp` and `mov ebp, esp` instructions to set up a stack frame, the `ebp` register (also known as the base pointer) points to the base of the current stack frame.

When you call a function in C, the following things happen:

1. The caller pushes the return address onto the stack.
2. The caller pushes any arguments onto the stack, in reverse order (i.e., the last argument is pushed first).
3. The caller calls the function using the `call` instruction, which pushes the current instruction pointer onto the stack.
4. The callee (the function being called) sets up its stack frame by pushing `ebp` onto the stack and moving `esp` into `ebp`.

So, when you look at the stack frame, it looks like this:

- `ebp` (base pointer) points to the base of the stack frame
- `ebp + 0` points to the saved `ebp` value (i.e., the `ebp` value of the caller)
- `ebp + 4` points to the return address
- `ebp + 8` points to the first argument passed to the function

In your case, the `idt_load` function takes a single argument, which is the address of the IDT. This argument is passed on the stack, and its address is `ebp + 8`.

The reason you see `[ebp+8]` in the assembly code is that the compiler is using the `ebp` register as a base pointer to access the argument that was passed on the stack. By adding 8 to `ebp`, you get the address of the first argument, which is the address of the IDT.

Here's a breakdown of what's on the stack:

- `ebp` (base pointer): points to the base of the stack frame
- `ebp + 0`: saved `ebp` value (i.e., the `ebp` value of the caller)
- `ebp + 4`: return address
- `ebp + 8`: first argument (address of the IDT)

So, when you do `mov ebx, [ebp+8]`, you're loading the address of the IDT into the `ebx` register, which is then used to load the IDT using the `lidt` instruction.
```
## 11.5 Makefile changes.
before compiling, we need to make some changes to our Makefile:
```
# Makefile
FILES = ... ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
        nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
        i686-elf-gcc $(INCLUDES) -I./src/idt $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/memory/memory.o: ./src/memory/memory.c
        i686-elf-gcc $(INCLUDES) -I./src/memory $(FLAGS) -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o
```
And now we compile and run!

![[Pasted image 20250511022620.png]]

Interrupt 0!

# 12. Implementing `in` and `out` instructions.
So, we know that the x86 architecture has this two instructions `in` and `out` for reading and outputting data to IO ports. But we'll want to use them in our C code, so we'll implement them.

## 12.1 io.h and io.asm
```
// io.h
#ifndef IO_H
#define IO_H

unsigned char insb(unsigned char port);
unsigned char insw(unsigned short port);

void outb(unsigned char port, unsigned char val);
void outw(unsigned short port, unsigned short val);

#endif
```

- `#ifndef IO_H`: standard header guard stuff.
- `#define IO_H`: standard header guard stuff.
- `unsigned char insb(unsigned char port);`: here we define the prototype for our C `insb` call, which will get us 1 byte from `port`.
- `unsigned short insw(unsigned short port);`: here we define the prototype for our C `insw` call, which will get us 2 byte from `port`.
- `void outb(unsigned char port, unsigned char val);`: here we define the prototype for our C `outb` call, which will send 2 bytes to `port`.
- `void outw(unsigned short port, unsigned short val);`: here we define the prototype for our C `outw` call, which will send 2 bytes to `port`.
- `#endif`: end of our header guard.
```
; io.asm
section .asm

global insb
global insw
global outb
global outw

insb:
        push ebp
        mov ebp, esp
        xor eax, eax ; xor eax, since we'll use it to return the io byte
        mov edx, [ebp+8]
        in al, dx
        pop ebp
        ret

insw:
        push ebp
        mov ebp, esp
        xor eax, eax
        mov edx, [ebp+8]
        in al, dx
        pop ebp
        ret

outb:
        push ebp
        mov ebp, esp
        mov eax, [ebp+12]
        mov edx, [ebp+8]
        out dx, al
        pop ebp
        ret

outw:
        push ebp
        mov ebp, esp
        mov eax, [ebp+12]
        mov edx, [ebp+8]
        out dx, al
        pop ebp
        ret
```
I will not explain the four function, just one for each.

- `insb:`: we define our input from port function.
- `push ebp`: stack setup.
- `mov ebp, esp`: stack setup.
- `xor eax, eax ; xor eax, since we'll use it to return the io byte`: here we zero out the `eax` register, which is used to return the value of the port.
- `mov edx, [ebp+8]`: here we move the `port` argument into the `edx` register. the lower 8 bits will have our argument, in this case `port`.
- `in al, dx`: here we make the `in` call, and receive the input from the port in the `al` register, or the lower eight bits of the `eax` register.
- `pop ebp`: here we pop the base pointer.
- `ret`: now we return to the calling function.


- `outb:`: here we define our output to port function.
- `push ebp`: stack setup.
- `mov ebp, esp`: stack setup.
- `mov eax, [ebp+12]`: here we move the value to be written `val` into the `eax` register.
- `mov edx, [ebp+8]`: and here we take the first argument `port` and set it to the `edx` register.
- `out dx, al`: here we make the call to `out`, and send the byte to the port.
- `pop ebp`: popping our base pointer.
- `ret`: we return to the calling function.

## 12.2 Makefile changes
We had to modify our Makefile once again.
```
FILES = ... ./build/io/io.asm.o
./build/io/io.asm.o: ./src/io/io.asm
        nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o
```
# 13. Programmable Interrupt Controller
- It allows hardware to interrupt the processor state, such as the keyboard, hard disk, mouse and more.
- It's programmable (duh).
- It requires interrupt acknowledgment.

## 13.1 IRQs
- IRQs are mapped to a starting interrupt, for example 0x20.
- IRQ 0 would then be interrupt 0x20.
- IRQ 1 would then be interrupt 0x21.
- IRQ 2 would then be interrupt 0x22.
By default, some of the IRQs are mapped to interrupts 8-15. This is a problem as these interrupts are reserved in protected mode for exceptions, so we are required to remap the PIC.
## 13.2 Master vs Slave
- The system has two IRQs: one for master ports and one for slave ports.
- The master handles IRQ 0-7.
- The slave handles IRQ 8-15.
## 13.3 Ports
- `0x20` and `0x21` = Master IRQs.
- `0xA0` and `0xA1` = Slave IRQs.

## 13.4 Remapping the Master PIC
```
; we havent set this code up, we're understanding the PIC, not implementing it!
setup_pic:
	; init some flags in the PICs
	mov al, 00010001b ; b4=1: init; b3=0: Edge; b1=0: Cascade; b0=1: Need 4th init setp
	out 0x20, al ; tell master

	mov al, 0x20 ; master IRQ should be on INT 0x20 (Just after Intel exceptions)
	mov 0x21, al

	mov al, 00000001b ; b4 = 0: FNM; b3-2=00: Master/Slave set by hardware; b1=0: Not AEOI; b0=1: x86 mode.
	out 0x20, al
```
## 13.5 Interrupt Acknowledgement
We must acknowledge the IRQ; otherwise, it will not interrupt us again.
```
// we havent set this code up, we're understanding the PIC, not implementing it!
// we acknoledge the PIC by sending it an EOI (End of Interrupt) command.
outb(PIC1, PIC_EOI);
// this table might be handy later:
// ISR Definitions
#define PIC1 0x20           // IO base address for master PIC.
#define PIC2 0xA0           // IO base address for slave PIC.
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_DATA PIC2
#define PIC2_DATA (PIC2+1)
#define PIC_EOI 0x20        // end of interrupt command.
```
# 14. Programmable Interrupt Controller Implementation
# 14.1 Remapping the PIC in `kernel.asm`
```
...
	mov al, 00010001b
	out 0x20, al
	mov al, 0x20
	out 0x21, al
	mov al, 00000000b
	out 0x21, al
...
```

- `mov al, 00010001b`: we set the bits for the PIC initialization mode.
- `out 0x20, al`: here we tell the master to set itself to init mode.
- `mov al, 0x20`: here we pass 0x20 to the `al` register.
- `out 0x21, al`: here we tell the master.
- `mov al, 00000000b`: here we enable 32 bit protected mode in the PIC.
- `out 0x21, al`: and we tell the master.

- explain changes to idt.c
