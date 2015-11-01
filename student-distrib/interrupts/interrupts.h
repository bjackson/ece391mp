/**
 * interrupts.h
 *
 * vim:ts=4 expandtab
 */
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "../x86_desc.h"
#include "../lib.h"
#include "../devices/i8259.h"

#define MAX_EXCEPTION_ISR 31

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

// Interrupt IDT entries
#define KEYBOARD_IDT      0x21
#define RTC_IDT           0x28

#define SYSCALL_IDT       0x80

// Keyboard constants
#define KEYBOARD_PORT 0x60
#define SCANCODE_MAX  0x80

// RTC constants
#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT  0x71

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

//
void keyboard_isr();

//
void rtc_isr();

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
extern void isr33();
extern void isr40();
extern void isr128();

/*
 *
 */
static const char* const exception_desc[32] = {
    "Divide By Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "RESERVED/UNUSED",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "RESERVED",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",
    "Virtualization Exception",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "Security Exception",
    "RESERVED"
};



#endif // INTERRUPTS_H
