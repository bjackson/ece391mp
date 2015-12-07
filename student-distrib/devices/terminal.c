/**
 * terminal.c
 * vim:ts=4 expandtab
 */
#include "terminal.h"

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

// Stores the pids for the main shell started when each terminal was first switched to
volatile uint32_t shell_pids[NUM_TERMINALS];

// Stores the pids for the active task of each terminal
volatile uint32_t active_pids[NUM_TERMINALS];

// Stores the index of the current terminal
volatile uint32_t current_terminal = 0;

// Stores the screen position for each terminal between task switches
volatile int switch_screen_pos_x[NUM_TERMINALS];
volatile int switch_screen_pos_y[NUM_TERMINALS];

/*
 * int32_t terminal_open(const uint8_t* filename)
 * Decsription: Opens a terminal
 * Inputs: filename - ignored
 * Outputs: 0 on success
 */
int32_t terminal_open(const uint8_t* filename) {
    memset(keyboard_buffers, 0x00, sizeof(keyboard_buffers));
    memset(read_buffers, 0x00, sizeof(read_buffers));

    int i;
    for(i = 0; i < NUM_TERMINALS; i++) {
        keyboard_buffer_indices[i] = 0;
        read_ready_flags[i] = 0;
        shell_pids[i] = 0;
        active_pids[i] = 0;
        switch_screen_pos_x[i] = 0;
        switch_screen_pos_y[i] = 0;
    }

    return 0;
}

/*
 * int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Decsription: Reads data into the terminal
 * Inputs: fd - file descriptor, buf - buffer, nbytes - max bytes to read
 * Outputs: -1 on failure, number of bytes read on success
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

/*
 * int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes)
 * Decsription: Write bytes ton the terminal
 * Inputs: fd - file descriptor, buf - buffer, nbytes - max bytes to read
 * Outputs: -1 on failure, number of bytes read on success
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

/*
 * int32_t terminal_close(int32_t fd)
 * Decsription: Closes the terminal
 * Inputs: fd - file descriptor
 * Outputs: -1 on failure always
 */
int32_t terminal_close(int32_t fd) {
  // Terminal should not be allowed to be closed.
  return -1;
}

/*
 * int32_t terminal_write_key(uint8_t key)
 * Decsription: Write data to the terminal buffer
 * Inputs: key - keyboard entry
 * Outputs: -1 on failure, 0 on success
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

/*
 * int32_t terminal_clear()
 * Decsription: Clears the terminal
 * Inputs: none
 * Outputs: none
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
    switch_screen_pos_x[t_idx] = 0;
    switch_screen_pos_y[t_idx] = 0;

    set_cursor(0, 0);
}

/**
 *
 */
void switch_active_terminal_screen(uint32_t old_pid, uint32_t new_pid) {
    if(old_pid == KERNEL_PID || new_pid == KERNEL_PID) {
        log(WARN, "Can't switch terminal while in pre-shell kernel!", "switch_active_terminal_screen");
        return;
    }

    pcb_t* old_pcb = get_pcb_ptr_pid(old_pid);
    pcb_t* new_pcb = get_pcb_ptr_pid(new_pid);

    uint32_t old_terminal = old_pcb->terminal_index;
    uint32_t new_terminal = new_pcb->terminal_index;

    if(old_terminal == new_terminal) {
        log(DEBUG, "No use switching to the same terminal screen", "switch_active_terminal_screen");
        return;
    }

    void* old_backing = (void*) (VIDEO + (FOUR_KB * (old_terminal + 1)));
    void* new_backing = (void*) (VIDEO + (FOUR_KB * (new_terminal + 1)));

    // Identity map pages for video memory backing stores
    mmap_pid(new_pid, old_backing, old_backing, ACCESS_SUPER);
    mmap_pid(new_pid, new_backing, new_backing, ACCESS_SUPER);
    mmap_pid(new_pid, ((void*) VIDEO), ((void*) VIDEO), ACCESS_SUPER);

    // Copy video mem from VIDEO to backing for old_pid
    memcpy(old_backing, ((void*) VIDEO), FOUR_KB);

    // Save screen position
    switch_screen_pos_x[old_terminal] = get_screen_x();
    switch_screen_pos_y[old_terminal] = get_screen_y();

    // Copy video mem from backing for new_pid to VIDEO
    memcpy(((void*) VIDEO), new_backing, FOUR_KB);

    // Reset the location of the cursor
    reset_screen_pos();

    // Unmap identity maped pages for video memory backing stores
    munmap_pid(new_pid, old_backing);
    munmap_pid(new_pid, new_backing);
}

