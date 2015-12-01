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

#define STDIN_FD  0
#define STDOUT_FD 1

#define FILE_ARRAY_SIZE 8

#define MAX_TASKS 2

#define KERNEL_PID 0

#define MAX_ARGS_LENGTH 128

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
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // from pusha
  uint32_t irq, error_code; // from isr_common
  uint32_t eip, cs, eflags, task_esp, ss; // from hardware
} task_registers_t;

typedef struct pcb_t {
    uint32_t pid;
    file_desc_t file_array[FILE_ARRAY_SIZE];
    uint32_t parent_pid;
    uint32_t old_esp;
    uint32_t old_ebp;
    uint8_t args[MAX_ARGS_LENGTH];
    struct pcb_t* parent;
    task_registers_t registers;
} pcb_t;

// File descriptor table used by the kernel (will probably be moved later)
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

// Table used to track which PIDs are in use
extern uint32_t pid_use_array[MAX_TASKS + 1];

// Returns the number of running tasks
// @return the number of running tasks
int32_t get_number_of_running_tasks(void);

//
void init_kernel_file_array();

//
pcb_t* init_pcb(uint32_t pid);

//
pcb_t* get_pcb_ptr();

pcb_t* get_pcb_ptr_for_pid(int32_t pid);

//
file_desc_t* get_file_array();

#endif // TASKS_H
