# interrupts.S - <description here>
# vim:ts=4

.text

# Macro simplifying generation of ISR linkage
# n: Index of the ISR
# ec: Boolean indicating whether the exception places an error code on the
#     stack. Default of 0 (no error code)
.macro ISR n ec=0
.globl isr\n
isr\n:
    cli
    .ifeq \ec
    pushl $0xdeadbeef
    .endif
    pushl $\n
    jmp isr_common
.endm

ISR 0 0
ISR 1 0
ISR 2 0
ISR 3 0
ISR 4 0
ISR 5 0
ISR 6 0
ISR 7 0
ISR 8 1
ISR 10 1
ISR 11 1
ISR 12 1
ISR 13 1
ISR 14 1
ISR 16 0
ISR 17 1
ISR 18 0
ISR 19 0

ISR 33 0
ISR 128 0

# void isr_common(uint32_t isr_index, uint32_t error_code);
#
# Function common to all ISRs
# Parameters:
#   isr_index: Index of the ISR
#   error_code: Optional error code pushed by some exceptions, 0 otherwise
# Returns: none
isr_common:
    popl    %eax                       # eax: ISR index
    popl    %ecx                       # ecx: Error code

    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ecx
    pushl   %eax
    call    isr_handler                # isr_handler(isr_index, error_code)
    add     $8, %esp

    sti                                # Re-enable interrupts

    leave
    iret
