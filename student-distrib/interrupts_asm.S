# interrupts.S - <description here>
# vim:ts=4

.text

.globl isr0
isr0:
    cli
    pushl $0
    jmp isr_common

.globl isr8
isr8:
    cli
    jmp isr_common

.globl isr10
isr10:
    cli
    jmp isr_common

.globl isr11
isr11:
    cli
    jmp isr_common

.globl isr12
isr12:
    cli
    jmp isr_common

.globl isr13
isr13:
    cli
    jmp isr_common

.globl isr14
isr14:
    cli
    jmp isr_common

.globl isr17
isr17:
    cli
    jmp isr_common

# Function common to all ISRs
isr_common:
    popl   %eax              # Remove error code
    #pushl  %ebp
    #movl   %esp, %ebp
    call isr_handler

    #sti

    #leave
    iret
