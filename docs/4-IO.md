# Description
This page details some information on the current state of I/O operations. What we have right now is not much. But hey, we're learning :)
# Related code
The related code can be found on the following directories:
- src/io/io.asm
- src/io/io.h
# Architecture
Most of the code we have as of right now is written in Assembly; this is because we cannot access I/O ports *directly* using Assembly in C. So, we create the labels in Assembly and then make those labels global for our C code to use.
## `outb` and `outw`
`outb` and `outw` are wrapper labels for our C code. They send data to the given I/O port. `outb` uses bytes and `outw` uses words.
```
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
## `insb` and `insw`
`insb` and `insw` are, again, wrapper labels for our C code. They receive data from the given I/O port and sent it back using the return (`eax`) register. As explained before, `insb` takes bytes and `insw` takes words.
```
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
```
