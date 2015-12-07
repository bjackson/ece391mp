/**
 * terminal.h
 * vim:ts=4 expandtab
 */
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "../types.h"
#include "../log.h"

#define KEYBOARD_BUFFER_SIZE 128

#define NUM_TERMINALS 3

// Stores the pid for the main shell started when each terminal was first switched to
extern volatile uint32_t shell_pids[NUM_TERMINALS];

// Stores the index of the current terminal
extern volatile uint32_t current_terminal;

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

#endif /* TERMINAL_H */
