# interrupts.S - <description here>
# vim:ts=4

.text

.globl isr0

isr0:
    iret
    cli
    #pushw $0
    jmp isr_common

isr_common:
    call isr_handler
    #sti
    iret
