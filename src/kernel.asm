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
	mov ebp, 0x00200000
	mov esp, ebp
	; end of register setup 
	; enabling the A20 line.
	in al, 0x92
	or al, 2
	out 0x92, al
	; end of enabling the A20 line.

	; remap the master PIC
	mov al, 00010001b ; init mode
	out 0x20, al
	mov al, 0x20 ; int 0x20 is where master ISR should start
	out 0x21, al

	mov al, 00000000b
	out 0x21, al
	; end of master PIC remap

	call kernel_main
	jmp $

times 512-($ - $$) db 0
