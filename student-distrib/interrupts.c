/**
 * interrupts.c - <description here>
 */

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "interrupts.h"

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
    set_int_entry(33, (uint32_t) isr33);
    set_int_entry(40, (uint32_t) isr40);
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
 * Halt the kernel upon an unhandled exception
 */
void haltOnException() {
  cli();
  printf("Halting the system!");
  asm volatile(".1: hlt; jmp .1;");
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
    // Handle exceptions differently
    if(isr_index <= MAX_EXCEPTION_ISR) {
        clear(); // Clear video memory
        printf("An exception has occurred. You're Fired!\n");
        printf("ISR: %d\n", isr_index);
        printf("Error: %x\n", error_code);
        printf("Cause: %s\n\n", exception_desc[isr_index]);
        haltOnException();
    } else if(isr_index == KEYBOARD_IDT) {
        keyboard_isr();
    } else if(isr_index == RTC_IDT) {
        rtc_isr();
    } else {
        printf("Exception/Interrupt not yet handled!");
        haltOnException();
    }
}

/**
 *
 */
void keyboard_isr() {
    // Lookup table for conversion from keyboard scan code to ASCII character
    static uint8_t scancodes[128] = {
        '$', '$',
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
        '$', '$',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
        '$', '$', '$', '$',
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
        '$', '$', '$', '$', '$',
        'z', 'x', 'c', 'v', 'b', 'n', 'm',
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$',
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$',
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$',
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$',
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$',
        '$', '$'
    };

    uint8_t scan_code = inb(KEYBOARD_PORT);

    // Only process 'break' codes for now
    if(scan_code >= SCANCODE_MAX) {
        scan_code -= SCANCODE_MAX;
        if(scancodes[scan_code] != '$') {
            printf("%c", scancodes[scan_code]);
        }
    }

    send_eoi(KEYBOARD_IRQ);
}

/**
 *
 */
void rtc_isr() {
    test_interrupts();

    // Select register C and throw away contents
    outb(0x0C, RTC_INDEX_PORT);
    inb(RTC_DATA_PORT);

    send_eoi(RTC_IRQ);
}

