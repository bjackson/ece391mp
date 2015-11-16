/**
 * terminal.h
 * vim:ts=4 expandtab
 */
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "../types.h"

#define KEYBOARD_BUFFER_SIZE 128

//
int32_t terminal_open(const uint8_t* filename);

//
int32_t terminal_close(int32_t fd);

//
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

//
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

//
int32_t terminal_write_key(uint8_t key);

//
void terminal_clear();

#endif /* TERMINAL_H */
