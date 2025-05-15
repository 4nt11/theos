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
