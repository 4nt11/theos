# Description
This page details some information on the current state of kernel and it's doings.
# Related code
The related code can be found on the following directories:
- src/kernel.asm
- src/kernel.c
- src/kernel.h
- src/memory/heap/kheap.c
- src/memory/heap/kheap.h
# Architecture
The kernel is composed by several parts of code. Most important ones are `kernel.asm` and `kernel.c`.
## `kernel.asm`
The `kernel.asm` binary gives execution to the C code in `kernel.c`. That's its purpose. It's initially loaded at `0x100000` in physical memory and then execution goes from there.
## Section setup
The `kernel.asm` file also sets up the main section registers (`ds, ss, fs, es, gs, ax`).
```
[BITS 32]
global _start
extern kernel_main
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
```
## A20 line
The `kernel.asm` file also enables de A20 line. This si to be able to access the entire memory address space.
```
        ; enabling the A20 line.
        in al, 0x92
        or al, 2
        out 0x92, al
        ; end of enabling the A20 line.
```
## PIC Master Remaping
The `kernel.asm` also remaps the master PIC. Doing this allows us to add our own interrupts later on.
```
        ; remap the master PIC
        mov al, 00010001b ; init mode
        out 0x20, al
        mov al, 0x20 ; int 0x20 is where master ISR should start
        out 0x21, al

        mov al, 00000000b
        out 0x21, al
        ; end of master PIC remap
```
## `kernel.asm -> kernel_main`
Finally, the `kernel.asm` calls `kernel_main`, which is in our `kernel.c` file.
## `kernel_main`
The `kernel_main` is the main execution thread of the kernel. For now, it doesn't do much.
## `terminal_initialize`
This function initializes (clears up) the screen after the kernel is loaded.
## `print(str)*`
The `print` function allows the caller to print a string to the console. This is done via VGA memory manipulation.
*\*This is a wrapper function.*
## `enable_interrupts`
This function is from the [[3. Interrupts]] section and it enables interrupts via the Assembly `sti` instruction.
## `kheap_init`
This functions is from the [[5. Memory]] section and it initializes the main heap of the kernel.
## `idt_init`
This function is from the [[3. Interrupts]] section and it initializes the [[Interrupt Descriptor Table Related Notes|Interrupt Descriptor Table]].
## `kernel.h`
The header file contains some prototypes, mainly the `print` and `kernel_main` functions. It also contains the `VGA_HEIGHT` and `VGA_WIDTH` constants.
```
#define VGA_WIDTH 80
#define VGA_HEIGHT 20

void kernel_main();
void print(const char* str);
```

- `VGA_WIDTH `
	- Description:
		- This the width of the VGA buffer, also known as `terminal_col`.
- `VGA_HEIGHT `
	- Description:
		- This is the height of the VGA buffer, also known as `terminal_row`.
