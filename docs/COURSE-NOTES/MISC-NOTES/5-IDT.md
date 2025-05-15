- [OsDev page on the Interrupt Descriptor Table](https://wiki.osdev.org/Interrupt_Descriptor_Table)

The **Interrupt Descriptor Table** (**IDT**) is a binary data structure specific to the [IA-32](https://wiki.osdev.org/IA32_Architecture_Family "IA32 Architecture Family") and [x86-64](https://wiki.osdev.org/X86-64 "X86-64") architectures. It is the [Protected Mode](https://wiki.osdev.org/Protected_Mode "Protected Mode") and Long Mode counterpart to the [Real Mode](https://wiki.osdev.org/Real_Mode "Real Mode") Interrupt Vector Table ([IVT](https://wiki.osdev.org/IVT "IVT")) telling the CPU where the [Interrupt Service Routines](https://wiki.osdev.org/Interrupt_Service_Routines "Interrupt Service Routines") (ISR) are located (one per interrupt vector). It is similar to the [Global Descriptor Table](https://wiki.osdev.org/Global_Descriptor_Table "Global Descriptor Table") in structure.

The IDT entries are called gates. It can contain Interrupt Gates, Task Gates and Trap Gates.

Before you implement the IDT, make sure you have a working GDT.

# IDTR Interrupt Descriptor Table Register
It's loaded using the `lidt` x86 Assembly instruction, whose argument is a pointer to and IDT Descriptor structure:

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

| 79 (64-bit Mode)  <br>48 (32-bit Mode)   16                | 15   0                     |
| ---------------------------------------------------------- | -------------------------- |
| **Offset**  <br>63 (64-bit Mode)  <br>31 (32-bit Mode)   0 | **Size**  <br>  <br>15   0 |
# Gate types

## Gate Types

There are basically two kinds of interrupts: ones that occur when code execution has encountered an **[Exception](https://wiki.osdev.org/Exceptions "Exceptions")** due to bad code, or ones that occur to handle events unrelated to currently executing code. In the first case it is pertinent to save the address of the _currently_ executing instruction so that it can be retried, these are called **Traps**. In the second case it is pertinent to save the address of the _next_ instruction so that execution can be resumed where it left off. These could be caused by an IRQ or other hardware event, or by use of the **INT** instruction. Another difference to note is that with **Traps**, new interrupts might occur during the service routine, but when the CPU is serving an IRQ, further interrupts are masked until an **End of Interrupt** signal is sent. How a certain interrupt is served depends on which kind of gate you put in the IDT entry.

### Interrupt Gate

An **Interrupt Gate** is used to specify an **[Interrupt Service Routine](https://wiki.osdev.org/Interrupt_Service_Routines "Interrupt Service Routines")**. For example, when the assembly instruction **INT 50** is performed while running in protected mode, the CPU looks up the 50th entry (located at 50 * 8) in the **IDT**. Then the Interrupt Gate's **Selector** and **Offset** values are loaded. The **Selector** and **Offset** are used to call the **Interrupt Service Routine**. When the **IRET** instruction is performed, the CPU returns from the interrupt. If the CPU was running in 32-bit mode and the specified selector is a 16-bit gate, then the CPU will go in 16-bit Protected Mode after calling the **ISR**. To return in this case, the **O32 IRET** instruction should be used, or else the CPU will not know that it should do a 32-bit return (reading 32-bit values off the [stack](https://wiki.osdev.org/Stack "Stack") instead of 16 bit).

### Trap Gate

A **Trap Gate** should be used to handle **[Exceptions](https://wiki.osdev.org/Exceptions "Exceptions")**. When such an exception occurs, there can sometimes be an error code placed on the stack, which should be popped before returning from the interrupt.

**Trap Gates** and **Interrupt Gates** are similar, and their descriptors are structurally the same, differing only in the **Gate Type** field. The difference is that for **Interrupt Gates**, interrupts are automatically disabled upon entry and reenabled upon **IRET**, whereas this does not occur for **Trap Gates**.

### Task Gate

A **Task Gate** is a gate type specific to IA-32 that is used for hardware task switching. For a **Task Gate** the **Selector** value should refer to a position in the **[GDT](https://wiki.osdev.org/GDT "GDT")** which specifies a **[Task State Segment](https://wiki.osdev.org/Task_State_Segment "Task State Segment")** rather than a code segment, and the **Offset** value is unused and should be set to zero. Rather than jumping to a service routine, when the CPU processes this interrupt, it will perform a hardware task switch to the specified task. A pointer back to the task which was interrupted will be stored in the **Task Link** field in the **TSS**.

| "*NOTE* Because IA-32 tasks are not re-entrant, an interrupt-handler task must disable interrupts between the time it completes handling the interrupt and the time it executes the IRET instruction. This action prevents another interrupt from occurring while the interrupt task's TSS is still marked busy, which would cause a general-protection (#GP) exception."<br><br>—Intel Software Developer Manual |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |

This type of gate is not often used as hardware task switching is slow and has little to no optimization on modern processors. As well, it has been entirely removed on [x86-64](https://wiki.osdev.org/X86-64 "X86-64").

| Name                        | Value         | Description                                                                                                                                                  |
| --------------------------- | ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 80386 32 bit Task Gate      | `0x05/0b0101` | Task gates reference TSS descriptors and can assist in multitasking when exceptions occur.                                                                   |
| 80386 16-bit Interrupt Gate | `0x06/0b0110` | Interrupt gates are to be used for interrupts that we want to invoke ourselves in our code.                                                                  |
| 80386 16-bit Trap Gate      | `0x07/0b0111` | Trap gates are like interrupt gates however they are used for exceptions. They also disable interrupts on entry and re-enable them on an "iret" instruction  |
| 80386 32-bit Interrupt Gate | `0x0E/0b1110` | Interrupts gates are to be used for interrupts that we want to invoke ourselves in our code.                                                                 |
| 80386 32-bit Trap Gate      | `0x0F/0b1111` | Trap gates are like interrupt gates however they are used for exceptions. They also disable interrupts on entry and re-enable them on an "iret" instruction. |
![[Pasted image 20250510221235.png]]
# 0xEE? Why?
`0xEE` is 11101110. And if we start checking bit by bit in relation to the attributes, from left to right:
- 2 bits for DPL - `11` is 3, so ring level 3, userspace ring level.
- 1 bit for interrupt set - `1` is `1`, so interrupt is set.
- 1 bit for storage seg bit - `0` is `0`, we set it to 0, because this is an interrupt gate.
- 4 bits for gate type - `1110` is for the 32-bit interrupt gate. look at the table above!

and `0xEE` = 11101110.

