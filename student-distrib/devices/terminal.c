/**
 * terminal.c
 * vim:ts=4 expandtab
 */
#include "terminal.h"
#include "../lib.h"
#include "../tasks.h"

// Holds the current line of input
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];

// Holds a copy of the previous line of input (after hitting enter)
static uint8_t read_buffer[KEYBOARD_BUFFER_SIZE];

// Tracks the index of the next character to be inserted
static uint32_t keyboard_buffer_index;

// Indicates whether the read_buffer is ready to be read from
static volatile uint8_t read_ready;

/**
 *
 */
int32_t terminal_open(const uint8_t* filename) {
    memset(keyboard_buffer, 0x00, sizeof(keyboard_buffer));
    memset(read_buffer, 0x00, sizeof(read_buffer));

    keyboard_buffer_index = 0;
    read_ready = 0;

    return 0;
}

/**
 *
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    if(fd != STDIN_FD) {
        printf("terminal_read: Invalid file descriptor\n");
        return -1;
    }

    memset(read_buffer, 0x00, sizeof(read_buffer));

    // Wait for read_ready to be set
    uint32_t spin = 0;
    while (!read_ready) {
        spin++;
    }

    int32_t bytes_to_read = (nbytes > KEYBOARD_BUFFER_SIZE) ?
        KEYBOARD_BUFFER_SIZE : nbytes;

    cli();
    int i;
    for(i = 0; i < bytes_to_read; i++) {
        uint8_t next = read_buffer[i];
        ((uint8_t*) buf)[i] = next;

        // Stop returning bytes after encountering a newline
        if(next == '\n') {
            read_ready = 0;
            sti();
            return i + 1;
        }
    }

    read_ready = 0;
    sti();

    return bytes_to_read;
}

/**
 *
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    if(fd != STDOUT_FD) {
        printf("terminal_write: Invalid file descriptor\n");
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

    // Handle backspace
    if(key == '\b') {
        cli();
        if(keyboard_buffer_index > 0) {
            putc('\b');
        }
        keyboard_buffer_index = (keyboard_buffer_index == 0) ? 0 :
            keyboard_buffer_index - 1;
        keyboard_buffer[keyboard_buffer_index] = 0x00;
        sti();
        return 0;
    }

    // Handle enter
    if(key == '\n') {
        cli();
        keyboard_buffer[keyboard_buffer_index] = '\n';
        memcpy(read_buffer, keyboard_buffer, sizeof(keyboard_buffer));
        memset(keyboard_buffer, 0x00, KEYBOARD_BUFFER_SIZE);
        keyboard_buffer_index = 0;
        putc('\n');
        read_ready = 1;
        sti();
        return 0;
    }

    // Save at least one character for the line feed
    if(keyboard_buffer_index == KEYBOARD_BUFFER_SIZE - 1) {
        return -1;
    }

    keyboard_buffer[keyboard_buffer_index++] = key;
    putc(key);
    return 0;
}

/**
 *
 */
void terminal_clear() {
    clear();

    memset(keyboard_buffer, 0x00, sizeof(keyboard_buffer));
    memset(read_buffer, 0x00, sizeof(read_buffer));

    keyboard_buffer_index = 0;
    read_ready = 0;
}

