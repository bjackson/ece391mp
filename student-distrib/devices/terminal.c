/**
 * terminal.c
 * vim:ts=4 expandtab
 */
#include "terminal.h"
#include "../lib.h"
#include "../tasks.h"

#define NUM_COLS 80
#define NUM_ROWS 25

// Holds the current line of input
static uint8_t keyboard_buffers[NUM_TERMINALS][KEYBOARD_BUFFER_SIZE];

// Holds a copy of the previous line of input (after hitting enter)
static uint8_t read_buffers[NUM_TERMINALS][KEYBOARD_BUFFER_SIZE];

// Tracks the index of the next character to be inserted
static uint32_t keyboard_buffer_indices[NUM_TERMINALS];

// Indicates whether the read_buffer is ready to be read from
static volatile uint8_t read_ready_flags[NUM_TERMINALS];

// Stores the pid for the main shell started when each terminal was first switched to
volatile uint32_t shell_pids[NUM_TERMINALS];

// Stores the index of the current terminal
volatile uint32_t current_terminal = 0;

/**
 *
 */
int32_t terminal_open(const uint8_t* filename) {
    memset(keyboard_buffers, 0x00, sizeof(keyboard_buffers));
    memset(read_buffers, 0x00, sizeof(read_buffers));

    int i;
    for(i = 0; i < NUM_TERMINALS; i++) {
        keyboard_buffer_indices[i] = 0;
        read_ready_flags[i] = 0;
        shell_pids[i] = 0;
    }

    return 0;
}

/**
 *
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    if(fd != STDIN_FD) {
        log(WARN, "Invalid file descriptor", "terminal_read");
        return -1;
    }

    pcb_t* pcb = get_pcb_ptr();
    if(pcb == NULL) {
        log(ERROR, "Can't call terminal functions before starting shell", "terminal_read");
        return -1;
    }
    uint32_t t_idx = pcb->terminal_index;

    memset(read_buffers[t_idx], 0x00, sizeof(read_buffers[t_idx]));

    // Wait for read_ready to be set
    uint32_t spin = 0;
    while (!read_ready_flags[t_idx]) {
        spin++;
    }

    int32_t bytes_to_read = (nbytes > KEYBOARD_BUFFER_SIZE) ?
        KEYBOARD_BUFFER_SIZE : nbytes;

    cli();
    int i;
    for(i = 0; i < bytes_to_read; i++) {
        uint8_t next = read_buffers[t_idx][i];
        ((uint8_t*) buf)[i] = next;

        // Stop returning bytes after encountering a newline
        if(next == '\n') {
            read_ready_flags[t_idx] = 0;
            sti();
            return i + 1;
        }
    }

    read_ready_flags[t_idx] = 0;
    sti();

    return bytes_to_read;
}

/**
 *
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    if(fd != STDOUT_FD) {
        log(WARN, "Invalid file descriptor", "terminal_write");
        return -1;
    }

    int i;
    for(i = 0; i < nbytes; i++) {
        uint8_t next = ((uint8_t*) buf)[i];
        putc(next);
    }

    return nbytes;
}

/**
 *
 */
int32_t terminal_close(int32_t fd) {
  // Terminal should not be allowed to be closed.
  return -1;
}

/**
 *
 */
int32_t terminal_write_key(uint8_t key) {
    pcb_t* pcb = get_pcb_ptr();
    if(pcb == NULL) {
        log(ERROR, "Can't call terminal functions before starting shell", "terminal_write_key");
        return -1;
    }
    uint32_t t_idx = pcb->terminal_index;

    // Handle backspace
    if(key == '\b') {
        cli();
        if(keyboard_buffer_indices[t_idx] > 0) {
            putc('\b');
        }
        keyboard_buffer_indices[t_idx] = (keyboard_buffer_indices[t_idx] == 0) ? 0 :
            keyboard_buffer_indices[t_idx] - 1;
        keyboard_buffers[t_idx][keyboard_buffer_indices[t_idx]] = 0x00;
        sti();
        return 0;
    }

    // Handle enter
    if(key == '\n') {
        cli();
        keyboard_buffers[t_idx][keyboard_buffer_indices[t_idx]] = '\n';
        memcpy(read_buffers[t_idx], keyboard_buffers[t_idx], sizeof(keyboard_buffers[t_idx]));
        memset(keyboard_buffers[t_idx], 0x00, KEYBOARD_BUFFER_SIZE);
        keyboard_buffer_indices[t_idx] = 0;
        putc('\n');
        read_ready_flags[t_idx] = 1;
        sti();
        return 0;
    }

    // Save at least one character for the line feed
    if(keyboard_buffer_indices[t_idx] == KEYBOARD_BUFFER_SIZE - 1) {
        return -1;
    }

    keyboard_buffers[t_idx][keyboard_buffer_indices[t_idx]++] = key;
    putc(key);
    return 0;
}

/**
 *
 */
void terminal_clear() {
    pcb_t* pcb = get_pcb_ptr();
    if(pcb == NULL) {
        log(ERROR, "Can't call terminal functions before starting shell", "terminal_clear");
        return;
    }
    uint32_t t_idx = pcb->terminal_index;

    // Will only clear the correct terminal due to video memory mapping
    clear();

    memset(keyboard_buffers[t_idx], 0x00, sizeof(keyboard_buffers[t_idx]));
    memset(read_buffers[t_idx], 0x00, sizeof(read_buffers[t_idx]));
    keyboard_buffer_indices[t_idx] = 0;
    read_ready_flags[t_idx] = 0;

    set_cursor(0, 0);
}

