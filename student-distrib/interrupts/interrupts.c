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

// Declared in terminal.c
extern volatile uint32_t shell_pids[NUM_TERMINALS];
extern volatile uint32_t active_pids[NUM_TERMINALS];

/*
 * init_idt()
 * Decsription: Initialize the IDT
 * Inputs: none
 * Outputs: none
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

/*
 * set_trap_entry(uint8_t idx, uint32_t handler)
 * Decsription: sets trap entries
 * Inputs: idx - idx, handler - the handler
 * Outputs: none
 */
void set_trap_entry(uint8_t idx, uint32_t handler) {
    set_idt_entry(idx, handler, 0xF, 0);
}

/*
 * set_int_entry(uint8_t idx, uint32_t handler)
 * Decsription: sets int entries
 * Inputs: idx - idx, handler - the handler
 * Outputs: none
 */
void set_int_entry(uint8_t idx, uint32_t handler) {
    set_idt_entry(idx, handler, 0xE, 0);
}

/*
 * set_sys_entry(uint8_t idx, uint32_t handler)
 * Decsription: sets system entries
 * Inputs: idx - idx, handler - the handler
 * Outputs: none
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

/*
 * set_idt_entry(uint8_t idx, uint32_t handler, uint8_t type, uint8_t dpl)
 * Decsription: sets idt entries
 * Inputs: idx - idx, handler - the handler, type - idt type, dpl - descriptor priviledge level
 * Outputs: none
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

/*
 * isr_handler(uint32_t isr_index, uint32_t error_code)
 * Decsription: Handler for ISR
 * Inputs: isr_index - index, error_code - error that occured
 * Outputs: none
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

/*
 * keyboard_isr()
 * Decsription: keyboard handling for the ISR
 * Inputs: none
 * Outputs: none
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
    uint8_t key = scancodes[scan_code];

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

    // On CTRL-l, clear the screen
    if (ctrl_pressed == 1 && key == 'l') {
        terminal_clear();
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    // On CTRL-c, halt the current task
    //TODO: CTRL-c for programs, CTRL-d for shells
    //TODO: Actually use signals lol jk
    if(ctrl_pressed == 1 && key == 'c') {
        send_eoi(KEYBOARD_IRQ);
        do_syscall(SYSCALL_HALT_NUM, 0, 0, 0);
        return;
    }

    // On CTRL-p, print the current running pid
    if(ctrl_pressed == 1 && key == 'p') {
        pcb_t* pcb = get_pcb_ptr();
        printf("Current PID: %d\n", (pcb == NULL) ? KERNEL_PID : pcb->pid);
        printf("Parent PID: %d\n", (pcb == NULL) ? KERNEL_PID : pcb->parent_pid);
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    // Support switching between terminals with ALT-F{1,2,3}
    if(alt_pressed == 1 && scan_code == F1) {
        log(DEBUG, "Switch to first terminal!", "isr");
        current_terminal = 0;
        send_eoi(KEYBOARD_IRQ);

        //TODO: Need to switch to the active task of each terminal, not the base shell

        if(shell_pids[0] > 0) {
            // A shell has already been started for this terminal
            log(DEBUG, "Shell already exists for terminal 0!", "isr");

            // Switch to base shell of terminal 0
            task_switch(active_pids[0]);
        } else {
            // Need to start a new shell for this terminal
            do_execute((uint8_t *)"shell");
        }

        return;
    } else if(alt_pressed == 1 && scan_code == F2) {
        log(DEBUG, "Switch to second terminal!", "isr");
        current_terminal = 1;
        send_eoi(KEYBOARD_IRQ);

        if(shell_pids[1] > 0) {
            // A shell has already been started for this terminal
            log(DEBUG, "Shell already exists for terminal 1!", "isr");

            // Switch to base shell of terminal 1
            task_switch(active_pids[1]);
        } else {
            // Need to start a new shell for this terminal
            do_execute((uint8_t *)"shell");
        }

        return;
    } else if(alt_pressed == 1 && scan_code == F3) {
        log(DEBUG, "Switch to third terminal!", "isr");
        current_terminal = 2;
        send_eoi(KEYBOARD_IRQ);

        if(shell_pids[2] > 0) {
            // A shell has already been started for this terminal
            log(DEBUG, "Shell already exists for terminal 2!", "isr");

            // Switch to base shell of terminal 2
            task_switch(active_pids[2]);
        } else {
            // Need to start a new shell for this terminal
            do_execute((uint8_t *)"shell");
        }

        return;
    }

    // Uppercase character if caps lock is on or a shift is pressed
    if(caps_lock || shift_bitmask) {
        key = upcase_char(key);
    }

    // Only process make (press) codes and handled codes
    if(scan_code < SCANCODE_MAX && scancodes[scan_code] != '$') {
        terminal_write_key(key);
    }

    send_eoi(KEYBOARD_IRQ);
}


/*
 * rtc_isr()
 * Decsription: isr handler for the rtc
 * Inputs: none
 * Outputs: none
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

/*
 * disable_inits()
 * Decsription: disable interrupts. used by rtc
 * Inputs: none
 * Outputs: none
 */
void disable_inits() {
    cli();

    // Disable NMI by setting the 0x80 bit
    uint8_t previous = inb(0x70);
    outb(previous | 0x80, 0x70);
}

/*
 * enable_inits()
 * Decsription: enable interrupts. used by rtc
 * Inputs: none
 * Outputs: none
 */
void enable_inits() {
    sti();

    // Re-enable NMI
    uint8_t previous = inb(0x70);
    outb(previous & 0x7F, 0x70);
}

/*
 * upcase_char(uint8_t character)
 * Decsription: handles upper case characters
 * Inputs: character - character to return the shift value off of
 * Outputs: the upper case version of the character
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
