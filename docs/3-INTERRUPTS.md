# Description
This page details the current state of interrupts in our kernel.
# Related code
The related code can be found on the following directories:
- src/idt
- src/idt/idt.asm
- src/idt/idt.c
- src/idt/idt.h
# Architecture
The IDT section is missing a lot of stuff. For now, it can only handle `int 21h` or a single keyboard press. Although the master PIC does receive constant acknoledgement.
## `idt_load`
The `idt.asm` file contains the label `idt_load` which, as the name implies, loads the IDT using the `lidt` instruction. It also sets up its own stack and the pops it.
```
idt_load:
        push ebp
        mov ebp, esp
        mov ebx, [ebp+8]
        lidt [ebx]
        pop ebp
        ret
```
This is done using an argument passed onto it by some code in the `idt.c` file.
## `enable_interrupts` and `disable_interrupts`
The `idt.asm` also creates some interfaces which can be used globally to enable and disable interrupts using `sti` and `cli` respectively.
```
enable_interrupts:
        sti
        ret

disable_interrupts:
        cli
        ret
```
## `int21h`
The `int21h` label in `idt.asm` handles the interrupt 21h, which is related to keyboard presses. It does nothing more than that, though, and since we don't have a proper keyboard driver, it only registers a single keypress.
```
int21h:
        cli
        pushad
        call int21h_handler
        popad
        sti
        iret
```
## `no_interrupt`
The `no_interrupt` label exists to be able to handle occasions where the system or the user don't send any interrupts at all. In real systems, this wouldn't happen as much, as operations are being done constantly. But in our case, we need to handle them as soon as possible. And we do.
```
no_interrupt:
        cli
        pushad
        call no_interrupt_handler
        popad
        sti
        iret
```
## `idt.h`
Before going into `idt.c`, I need to explain the structures that are being used _in_ the `idt.c` file. Without them, we won't understand the code.
### `idt_desc`
First we have the `idt_desc` structure, which is our proper [[Interrupt Descriptor Table Related Notes|Interrupt Descriptor Table]].

*\*Many things are not well described here. It's not the purpose of this documentation to explain what each thing is and what it does. Check the related page for more info.*

```
struct idt_desc
{
        uint16_t offset_1; // offset bits 0 - 15
        uint16_t selector; // selector in our GDT
        uint8_t zero;      // does nothing; bits are reserved.
        uint8_t type_attr; // descriptor type and attributes.
        uint16_t offset_2; // offset bits 16-31

} __attribute__((packed));
```
- `offset_1`
	- Type:
		- uint16_t 
	- Description:
		- This variable is the initial section of our offset, which is divided in two parts.
- `selector`
	- Type:
		- uint16_t 
	- Description:
		- This is the `selector` section, in which we set the `CODE_SEG` selector from our GDT.
- `zero`
	- Type:
		- uint8_t
	- Description:
		- Unused. Must be zero.
- `type_attr`
	- Type:
		- uint8_t 
	- Description:
		- Here go the interrupt descriptor attributes and type.
- `offset_2`
	- Type:
		- uint16_t 
	- Description: And finally we last bits of our offset.

### `idtr_desc`
The IDT also needs the IDT Register, which is built in this struct.
```
struct idtr_desc
{
        uint16_t limit;    // size of the descriptor table - 1
        uint32_t base;     // base address of the start of the interrupt table.
} __attribute__((packed));
```


- `limit`
	- Type
		- uint16_t 
	- Description:
		- This is the size of the IDT minus one.
- `base;`
	- Type:
		- uint32_t 
	- Description:
		- Base address of the GDT.
## `idt.c`
Now we can jump into the C code.
### `idt_descriptors[PEACHOS_TOTAL_INTERRUPTS]`
This is the basis of our IDT. Our system will have 512 interrupts (as per `PEACHOS_TOTAL_INTERRUPTS` in [[6. Configuration|config.h]]).
### `idtr_descriptor`
This is the IDT register.
### `int21h_handler`
This is a simple function that prints out `kb pressed!` when the keyboard is pressed and then acknowledges the PIC by using the `outb` instruction implemented and documented in the [[4. IO Operations|I/O section]].
### `no_interrupts`
This function continually acknoledges the PIC.
### `idt_zero`
This function just prints `divide by zero error` as per Intel's documentation and reserved interrupts. More on [[Interrupt Descriptor Table Related Notes]].
### `idt_set`
The `idt_set` function set up the interrupt descriptor table.
```
void idt_set(int interrupt_no, void* addr)
{
        struct idt_desc* desc = &idt_descriptors[interrupt_no];
        desc->offset_1 = (uint32_t) addr & 0x0000ffff;
        desc->selector = KERNEL_CODE_SELECTOR;
        desc->zero = 0x00;
        desc->type_attr = 0xEE;
        desc->offset_2 = (uint32_t) addr >> 16;
}
```
For more information about this values, check the section on the IDT in the [[3. Protected mode development#11. Implementing the IDT in our code.|implementing IDT]] section.
### `idt_init`
This function is called by the `kernel.c` [[2. Kernel#`idt_init`|routine]]. It's used to setup everything related to the IDT and its interrupts.
```
void idt_init()
{
        memset(idt_descriptors, 0, sizeof(idt_descriptors));
        idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
        idtr_descriptor.base = (uint32_t) idt_descriptors;
        for (int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++)
        {
                idt_set(i, no_interrupt);
        }
        idt_set(0, idt_zero);
        idt_set(0x21, int21h);

        // load interrupt descriptor table
        idt_load(&idtr_descriptor);
}
```

*\*`memset` is part of the [[5. Memory|memory]] section of the documentation.*
### `idt_load`
This is where we make use of the Assembly label that we saw [[#`idt_load`|`idt.asm` section]] . Here we pass the IDT descriptor for it to be loaded by `lidt`.
