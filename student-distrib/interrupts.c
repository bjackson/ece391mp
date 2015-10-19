/**
 * interrupts.c - <description here>
 */
#include "interrupts.h"

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
