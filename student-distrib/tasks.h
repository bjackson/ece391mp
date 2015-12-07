/**
 * tasks.h
 *
 * vim:ts=4 expandtab
 */
#ifndef TASKS_H
#define TASKS_H

#include "types.h"
#include "lib.h"
#include "devices/terminal.h"
#include "x86_desc.h"
#include "paging.h"
#include "devices/i8259.h"

#define STDIN_FD  0
#define STDOUT_FD 1

#define FILE_ARRAY_SIZE 8

#define MAX_TASKS 6

#define KERNEL_PID 0

#define MAX_ARGS_LENGTH 128

#define SWITCH_SCREEN 1
#define NO_SWITCH_SCREEN 0

// Struct for file descriptor array entry
typedef struct {
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    uint32_t inode_num;
    uint32_t file_pos;
    uint32_t flags;
} file_desc_t;

typedef struct {
    uint32_t pid;
    file_desc_t file_array[FILE_ARRAY_SIZE];
    uint32_t parent_pid;
    uint32_t parent_esp;
    uint32_t parent_ebp;
    uint8_t args[MAX_ARGS_LENGTH];
    uint32_t terminal_index;
    uint32_t from_task_switch;
    uint32_t switch_esp;
    uint32_t switch_ebp;
} pcb_t;

// File descriptor table used by the kernel (will probably be moved later)
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

// Table used to track which PIDs are in use
extern uint32_t pid_use_array[MAX_TASKS + 1];

// initialize the kernel file array
void init_kernel_file_array();

// initiliaze the pbc
pcb_t* init_pcb(uint32_t pid);

// get the pcb pointer
pcb_t* get_pcb_ptr();

// the the pcb pointer to the pid
pcb_t* get_pcb_ptr_pid(uint32_t pid);

// get file array
file_desc_t* get_file_array();

// switch tasks
void task_switch(uint32_t new_pid, uint32_t switch_screen);

//
void task_sched_next();
>>>>>>> Try to get scheduling working... It doesn't now.

#endif // TASKS_H
