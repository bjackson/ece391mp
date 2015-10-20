/**
 * interrupts.h
 */
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "x86_desc.h"
#include "lib.h"

typedef union seg_sel_t {
    uint16_t val;
    struct {
        uint16_t rpl:2;
        uint16_t ti:1;
        uint16_t index:13;
    } __attribute__((packed));
} seg_sel_t;

//
void init_idt();

//
void set_idt_entry(uint8_t idx, uint32_t handler);

//
extern void isr_handler();

// Interrupt handler functions - in interrupts_asm.S
extern void isr0();

#endif // INTERRUPTS_H
