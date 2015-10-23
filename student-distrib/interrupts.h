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
void set_trap_entry(uint8_t idx, uint32_t handler);

//
void set_int_entry(uint8_t idx, uint32_t handler);

//
void set_sys_entry(uint8_t idx, uint32_t handler);

//
void set_idt_entry(uint8_t idx, uint32_t handler, uint8_t type, uint8_t dpl);

//
extern void isr_handler(uint32_t isr_index, uint32_t error_code);

// Interrupt handler functions - in interrupts_asm.S
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr128();

#endif // INTERRUPTS_H
