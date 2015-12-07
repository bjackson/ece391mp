/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 expandtab
 */

#include "i8259.h"

#define I8259_INTMASK 0xff

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

// i8259_init
// Initialize the 8259 PIC
// @return nothing
void i8259_init(void) {
    // Mask all interrupts
    outb(I8259_INTMASK, MASTER_DATA);
    outb(I8259_INTMASK, SLAVE_DATA);

    // Start initialization sequence
    outb(ICW1, MASTER_COMMAND);
    outb(ICW1, SLAVE_COMMAND);

    // ICW2: Vector offset
    outb(ICW2_MASTER, MASTER_DATA);
    outb(ICW2_SLAVE, SLAVE_DATA);

    // ICW3: Master/Slave connection
    outb(ICW3_MASTER, MASTER_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA);

    // ICW4: Additional info
    outb(ICW4, MASTER_DATA);
    outb(ICW4, SLAVE_DATA);

    // Mask all interrupts (again)
    outb(0xff, MASTER_DATA);
    outb(0xff, SLAVE_DATA);

    /*
    // Initialize the PIT to the correct frequency
    outb(0x43, 0x34);
    outb(0x40, (500 & 0xFF));
    outb(0x40, (500 >> 8));
    */
}

// enable_irq
// Enable (unmask) the specified IRQ
// @param irq_num The IRQ to be enabled
// @return nothing
void enable_irq(uint32_t irq_num) {
    if(irq_num < 8) {
        master_mask = inb(MASTER_DATA);
        outb((master_mask & ~(1 << irq_num)), MASTER_DATA);
    } else if(irq_num < 16) {
        slave_mask = inb(SLAVE_DATA);
        outb((master_mask & ~(1 << (irq_num - 8))), SLAVE_DATA);
    }
}

// disable_irq
// Disable (mask) the specified IRQ
// @param irq_num The IRQ to be disabled
// @return nothing
void disable_irq(uint32_t irq_num) {
    if(irq_num < 8) {
        master_mask = inb(MASTER_DATA);
        outb((master_mask | (1 << irq_num)), MASTER_DATA);
    } else if(irq_num < 16) {
        slave_mask = inb(SLAVE_DATA);
        outb((master_mask | (1 << (irq_num - 8))), SLAVE_DATA);
    }
}

// send_eoi
// Send end-of-interrupt signal for the specified IRQ
// @param irq_num The IRQ to send the EOI to
// @return nothing
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8) {
        outb(EOI | (irq_num - 8), SLAVE_COMMAND);
        outb(EOI | SLAVE_IRQ, MASTER_COMMAND);
    } else {
        outb(EOI | irq_num, MASTER_COMMAND);
    }
}
