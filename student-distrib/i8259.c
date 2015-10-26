/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define I8259_INTMASK 0xff

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
	/**
	 * Note: All of this is currently untested
	 */

	// Save masks
	master_mask = inb(MASTER_DATA);
	slave_mask = inb(SLAVE_DATA);

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

	// Restore saved masks
	//outb(master_mask, MASTER_DATA);
	//outb(slave_mask, SLAVE_DATA);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {

}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

}
