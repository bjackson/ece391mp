/**
 * terminal.h
 * vim:ts=4 expandtab
 */
#ifndef _TERMINAL_H
#define _TERMINAL_H

#define NUM_TERMINALS 3
#define KEYBOARD_BUFFER_SIZE 128

#include "../types.h"
#include "../log.h"
#include "../lib.h"
#include "../tasks.h"

// open the terminal
int32_t terminal_open(const uint8_t* filename);

// doesn't allow closing the terminal
int32_t terminal_close(int32_t fd);

// read from the terminal
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

// write data to the terminal
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

// write keystrokes to the terminal
int32_t terminal_write_key(uint8_t key);

// clear the terminal
void terminal_clear();

// switch the active terminal
void switch_active_terminal_screen(uint32_t old_pid, uint32_t new_pid);

#endif /* TERMINAL_H */
