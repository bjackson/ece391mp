#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "../types.h"

#define KEYBOARD_BUFFER_SIZE 128

extern uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
extern uint8_t read_buffer[KEYBOARD_BUFFER_SIZE];

extern uint32_t keyboard_buffer_index;

int32_t terminal_open(const uint8_t* filename);

int32_t terminal_close(int32_t fd);

int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes);

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t backspace(void);

#endif
