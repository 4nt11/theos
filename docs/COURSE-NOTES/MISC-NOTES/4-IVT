# The Interrupt Vector Table
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

# Implementing our own interrupts in realmode.
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

