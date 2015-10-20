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
#define RESERVED_IDT      0x0F
#define MATHFAULT_IDT     0x10
#define ALIGNCHECK_IDT    0x11
#define MACHINECHECK_IDT  0x12
#define SIMDFLTPTEX_IDT   0x13

/**
 *
 */
void init_idt() {
    memset(&idt, 0x00, sizeof(idt_desc_t) * NUM_VEC);

    // Initialize all IDT entries
    int i;
    for(i = 0; i < NUM_VEC; i++) {
        set_idt_entry(i, (uint32_t) isr0);
    }

}

/**
 *
 */
void set_idt_entry(uint8_t idx, uint32_t handler) {
    seg_sel_t selector;
    selector.rpl = 0x3;         // Requested priviledge level (3)
    selector.ti = 0;            // Table index (GDT)
    selector.index = KERNEL_CS; // Segment index (Kernel code segment)

    idt_desc_t entry;

    SET_IDT_ENTRY(entry, handler);
    entry.seg_selector = selector.val;
    entry.type = 0xF;  // Type (32-bit trap gate)
    entry.ss = 0;      // Storage Segment
    entry.dpl = 0;     // Descriptor Priviledge Level
    entry.present = 1; // Present

    idt[idx] = entry;
}

extern void isr_handler() {
    printf("TRUMP SAYS NO!\n");
}
