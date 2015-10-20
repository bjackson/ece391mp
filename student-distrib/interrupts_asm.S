# interrupts.S - <description here>
# vim:ts=4

.text

.globl isr0

isr0:
    cli
    pushl $0
    pushl $0
    jmp isr_common

isr_common:
    popl  %eax
    popl  %eax
    call isr_handler
    sti
    iret
