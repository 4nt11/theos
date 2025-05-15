ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

_start: 
	jmp short goto_prot
	nop
times 33 db 0

goto_prot:
	jmp 0:prot_setup

prot_setup:
	cli
	mov ax, 0x00
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov sp, 0x7c00
	sti

.load_prot:
	cli
	lgdt [gdt_descriptor]
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax
	jmp CODE_SEG:prot_mode

gdt_start:
gdt_null:
	dd 0x0
	dd 0x0

; offset 0x08
gdt_code:
	dw 0xffff
	dw 0
	db 0
	db 0x9a
	db 11001111b
	db 0

; offset 0x10
gdt_data:
	dw 0xffff
	dw 0
	db 0
	db 0x92
	db 11001111b
	db 0
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start-1
	dd gdt_start

; now in protected mode :)
[BITS 32]
prot_mode:
	jmp $

times 510 - ($ - $$) db 0

dw 0xAA55


