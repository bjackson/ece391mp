/**
 * terminal.c
 * vim:ts=4 expandtab
 */
#include "terminal.h"
#include "../lib.h"

static volatile uint8_t read_ready;
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint8_t read_buffer[KEYBOARD_BUFFER_SIZE];

// Tracks the index of the next character to be inserted
static uint32_t keyboard_buffer_index;

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
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
    // Clear read buffer from previous calls
    memset(read_buffer, 0x00, sizeof(read_buffer));

    memset(buf, '\0', nbytes);

    uint32_t bufferIndex = 0;

    uint32_t spin = 0;
    while (readyToRead != TRUE) {
        spin++;
    }

    while (bufferIndex <= KEYBOARD_BUFFER_SIZE - 1 && bufferIndex < nbytes) {
        read_buffer[bufferIndex] = keyboard_buffer[bufferIndex];
        buf[bufferIndex] = keyboard_buffer[bufferIndex];
        if (keyboard_buffer[bufferIndex] == '\n') {
            bufferIndex++;
            break;
        }
        bufferIndex++;
    }
    keyboard_buffer_index = 0;
    readyToRead = 0;
    memset(keyboard_buffer, '\0', 128); // Clear keyboard buffer for new input
    return bufferIndex;
}
 */

/**
 *
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    int i;
    for(i = 0; i < nbytes; i++) {
        uint8_t next = ((uint8_t*) buf)[i];

        // Handle backspace
        if(next == '\b') {
            cli();
            keyboard_buffer_index = (keyboard_buffer_index == 0) ? 0 :
                --keyboard_buffer_index;
            keyboard_buffer[keyboard_buffer_index] = 0x00;
            putc('\b');
            sti();
            continue;
        }

        // Handle enter
        if(next == '\n') {
            cli();
            keyboard_buffer[keyboard_buffer_index] = '\n';
            memcpy(read_buffer, keyboard_buffer, sizeof(keyboard_buffer));
            memset(keyboard_buffer, 0x00, KEYBOARD_BUFFER_SIZE);
            keyboard_buffer_index = 0;
            putc('\n');
            read_ready = 1;
            sti();
            continue;
        }

        // Save at least one character for the line feed
        if(keyboard_buffer_index == KEYBOARD_BUFFER_SIZE - 1) {
            return i;
        }

        keyboard_buffer[keyboard_buffer_index++] = next;
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
void terminal_clear() {
    clear();

    memset(keyboard_buffer, 0x00, sizeof(keyboard_buffer));
    memset(read_buffer, 0x00, sizeof(read_buffer));

    keyboard_buffer_index = 0;
    read_ready = 0;
}

