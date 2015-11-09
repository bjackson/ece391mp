/**
 * interrupts.c
 *
 * vim:ts=4 expandtab
 */
#include "interrupts.h"
#include "../lib.h"
#include "../types.h"
#include "../devices/terminal.h"
#include "../devices/rtc.h"

// Indicates whether these keys were pressed
uint8_t ctrl_pressed  = 0;
uint8_t alt_pressed   = 0;
uint8_t caps_lock     = 0;
uint8_t shift_bitmask = 0; // Bit 1 for left, Bit 0 for right

/**
 *
 */
void init_idt() {
    memset(idt, 0x00, sizeof(idt_desc_t) * NUM_VEC);

    // Non-User defined exceptions and interrupts
    set_trap_entry(DIVBYZERO_IDT,   (uint32_t) isr0);
    set_trap_entry(DEBUGGER_IDT,    (uint32_t) isr1);
    set_int_entry(NMI_IDT,          (uint32_t) isr2);
    set_sys_entry(BREAKPOINT_IDT,   (uint32_t) isr3);
    set_sys_entry(OVERFLOW_IDT,     (uint32_t) isr4);
    set_sys_entry(BOUNDS_IDT,       (uint32_t) isr5);
    set_trap_entry(INVOPCODE_IDT,   (uint32_t) isr6);
    set_trap_entry(COPRUNAVAIL_IDT, (uint32_t) isr7);
    set_trap_entry(DBLFAULT_IDT,    (uint32_t) isr8);

    set_trap_entry(INVTASKSTS_IDT,  (uint32_t) isr10);
    set_trap_entry(SEGNPRESENT_IDT, (uint32_t) isr11);
    set_trap_entry(STACKFAULT_IDT,  (uint32_t) isr12);
    set_trap_entry(GPROTFAULT_IDT,  (uint32_t) isr13);
    set_int_entry(PAGEFAULT_IDT,    (uint32_t) isr14);
    set_trap_entry(MATHFAULT_IDT,   (uint32_t) isr16);
    set_trap_entry(ALIGNCHECK_IDT,  (uint32_t) isr17);
    set_trap_entry(MACHINECHECK_IDT,(uint32_t) isr18);
    set_trap_entry(SIMDFLTPTEX_IDT, (uint32_t) isr19);

    // User defined interrupts
    set_int_entry(KEYBOARD_IDT, (uint32_t) isr33);
    set_int_entry(RTC_IDT, (uint32_t) isr40);
    set_sys_entry(SYSCALL_IDT, (uint32_t) isr128);
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
  asm volatile(".1hltex: hlt; jmp .1hltex;");
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

/**
 *
 */
extern void isr_handler(uint32_t isr_index, uint32_t error_code) {
    // Handle exceptions differently
    if(isr_index <= MAX_EXCEPTION_ISR) {
        clear(); // Clear video memory
        printf("An exception has occurred. You're Fired!\n");
        printf("ISR: %d\n", isr_index);
        if(error_code != 0xDEADBEEF) {
            printf("Error: 0x%x\n", error_code);
        }
        printf("Cause: %s\n\n", exception_desc[isr_index]);

        // Page-Fault specific
        if(isr_index == PAGEFAULT_IDT) {
            asm volatile("movl %%cr2, %%eax" : : : "eax");
            register uint32_t *addr asm("eax");
            printf("Address: 0x%x\n", addr);

            printf("Reason: %s\n", (error_code & 0x1) ? "Page-level protection violation" :
                    "Non-present page");
            printf("R/W: %s\n", (error_code & 0x2) ? "Write" : "Read");
            printf("U/S: %s\n", (error_code & 0x4) ? "User mode" : "Supervisor mode");
            if(error_code & 0x8) {
                printf("Caused by reserved bits set to 1 in a page directory\n\n");
            } else {
                printf("\n");
            }
        }

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
        '$', '$', // Keyboard error, ESC
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        '$', // 0x1D
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
        '`', '$', '$',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
        '$', '$', '$',
        ' ',
        '$', '$', '$', '$', '$', '$', '$', '$', // 0x41
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$',
        '+',
        '$', '$', // 0x50
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', // 0x5F
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', // 0x6E
        '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', '$', // 0x7D
        '$', '$' // 0x7F
    };

    uint8_t scan_code = inb(KEYBOARD_PORT);

    switch (scan_code) {
      case RIGHT_SHIFT_PRESS:
        shift_bitmask |= 0x1;
        break;
      case LEFT_SHIFT_PRESS:
        shift_bitmask |= 0x2;
        break;
      case RIGHT_SHIFT_RELEASE:
        shift_bitmask &= 0xE;
        break;
      case LEFT_SHIFT_RELEASE:
        shift_bitmask &= 0xD;
        break;
      case ALT_PRESS:
        alt_pressed = 1;
        break;
      case ALT_RELEASE:
        alt_pressed = 0;
        break;
      case CONTROL_PRESS:
        ctrl_pressed = 1;
        break;
      case CONTROL_RELEASE:
        ctrl_pressed = 0;
        break;
      case CAPS_LOCK_PRESS:
        caps_lock = (caps_lock) ? 0 : 1; // Toggle caps lock
        break;
    }

    uint8_t key = scancodes[scan_code];

    // On CTRL-L, clear the screen
    if (ctrl_pressed == 1 && key == 'l') {
        terminal_clear();
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    // Uppercase character if caps lock is on or a shift is pressed
    if(caps_lock || shift_bitmask) {
        key = upcase_char(key);
    }

    // Only process make (press) codes and handled codes
    if(scan_code < SCANCODE_MAX && scancodes[scan_code] != '$') {
        terminal_write(STDOUT_FD, &key, 1);
    }

    send_eoi(KEYBOARD_IRQ);
}


/**
 *
 */
void rtc_isr() {
    // test_interrupts();

    // Select register C and throw away contents
    outb(0x0C, RTC_INDEX_PORT);
    inb(RTC_DATA_PORT);

    // increment tick counter
    tick_counter++;

    send_eoi(RTC_IRQ);
}

/**
 * Disable interrupts. Used by RTC
 */
void disable_inits() {
    cli();

    // Disable NMI by setting the 0x80 bit
    uint8_t previous = inb(0x70);
    outb(previous | 0x80, 0x70);
}

/**
 * Reenable interrupts. Used by RTC
 */
void enable_inits() {
    sti();

    // Re-enable NMI
    uint8_t previous = inb(0x70);
    outb(previous & 0x7F, 0x70);
}

/**
 *
 */
uint8_t upcase_char(uint8_t character) {
    // Upcase alphas mathematically
    if (character >= 97 && character <= 122) {
        return character - ('a' - 'A');
    }

    // Upcase the rest with a map
    switch (character) {
        case '=':
            return '+';
        case '-':
            return '_';
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':
            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        case '0':
            return ')';
        case '[':
            return '{';
        case ']':
            return '}';
        case '\\':
            return '|';
        case ';':
            return ':';
        case '\'':
            return '"';
        case ',':
            return '<';
        case '.':
            return '>';
        case '/':
            return '?';
        case '`':
            return '~';
        default:
            return character;
    }
}
