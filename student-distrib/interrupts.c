/**
 * interrupts.c - <description here>
 */

#include "x86_desc.h"
#include "lib.h"
#include "interrupts.h"

// Exception IDT entries
#define DIVBYZERO_IDT     0x00
#define DEBUGGER_IDT      0x01
#define NMI_IDT           0x02
#define BREAKPOINT_IDT    0x03
#define OVERFLOW_IDT      0x04
#define BOUNDS_IDT        0x05
#define INVOPCODE_IDT     0x06
#define COPRUNAVAIL_IDT   0x07
#define DBLFAULT_IDT      0x08
#define CPRSEGOVER_IDT    0x09
#define INVTASKSTS_IDT    0x0A
#define SEGNPRESENT_IDT   0x0B
#define STACKFAULT_IDT    0x0C
#define GPROTFAULT_IDT    0x0D
#define PAGEFAULT_IDT     0x0E
#define MATHFAULT_IDT     0x10
#define ALIGNCHECK_IDT    0x11
#define MACHINECHECK_IDT  0x12
#define SIMDFLTPTEX_IDT   0x13

/**
 *
 */
void init_idt() {
    memset(idt, 0x00, sizeof(idt_desc_t) * NUM_VEC);

    // Non-User defined exceptions and interrupts
    set_trap_entry(0, (uint32_t) isr0);
    set_trap_entry(1, (uint32_t) isr1);
    set_int_entry(2, (uint32_t) isr2);
    set_sys_entry(3, (uint32_t) isr3);
    set_sys_entry(4, (uint32_t) isr4);
    set_sys_entry(5, (uint32_t) isr5);
    set_trap_entry(6, (uint32_t) isr6);
    set_trap_entry(7, (uint32_t) isr7);
    set_trap_entry(8, (uint32_t) isr8);
    set_trap_entry(10, (uint32_t) isr10);
    set_trap_entry(11, (uint32_t) isr11);
    set_trap_entry(12, (uint32_t) isr12);
    set_trap_entry(13, (uint32_t) isr13);
    set_int_entry(14, (uint32_t) isr14);
    set_trap_entry(16, (uint32_t) isr16);
    set_trap_entry(17, (uint32_t) isr17);
    set_trap_entry(18, (uint32_t) isr18);
    set_trap_entry(19, (uint32_t) isr19);

    // User defined interrupts
    set_sys_entry(128, (uint32_t) isr128);
}

/**
 *
 */
void set_trap_entry(uint8_t idx, uint32_t handler) {
    set_idt_entry(idx, handler, 0xF, 0);
}

/**
 *
 */
void set_int_entry(uint8_t idx, uint32_t handler) {
    set_idt_entry(idx, handler, 0xE, 0);
}

/**
 *
 */
void set_sys_entry(uint8_t idx, uint32_t handler) {
    set_idt_entry(idx, handler, 0xF, 3);
}

/**
 *
 */
void set_idt_entry(uint8_t idx, uint32_t handler, uint8_t type, uint8_t dpl) {
    seg_sel_t selector;
    selector.rpl = 0;           // Requested priviledge level (0)
    selector.ti = 0;            // Table index (GDT)
    selector.index = 2;         // Segment index (Kernel code segment)

    idt_desc_t entry;
    memset(&entry, 0x00, sizeof(idt_desc_t));

    SET_IDT_ENTRY(entry, handler);
    entry.seg_selector = selector.val;
    entry.type = type;  // Type (32-bit interrupt gate)
    entry.dpl = dpl;     // Descriptor Priviledge Level
    entry.present = 1; // Present
    idt[idx] = entry;
}

extern void isr_handler(uint32_t isr_index, uint32_t error_code) {
    printf("An exception has occurred. You're Fired!\n");
    printf("ISR: %d\n", isr_index);
    printf("Error: %x\n", error_code);

    switch(isr_index) {
        case DIVBYZERO_IDT:
            printf("Cause: Divide by Zero\n\n");
            break;
        case GPROTFAULT_IDT:
            printf("Cause: General Protection Fault\n\n");
            break;
        default:
            printf("Not yet handled!");
    }
}
