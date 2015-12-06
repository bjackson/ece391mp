/**
 * tasks.c
 *
 * vim:ts=4 expandtab
 */
#include "tasks.h"

// File descriptor table used by the kernel (will probably be moved later)
file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

// If PID in use, pid_use_array[pid] = 1, 0 otherwise
uint32_t pid_use_array[MAX_TASKS + 1] = {0};

/**
 *
 */
void init_kernel_file_array() {
    // Initialize kernel stdin file desctriptor
    file_desc_t stdin_kernel;
    memset(&stdin_kernel, 0x00, sizeof(file_desc_t));
    stdin_kernel.read = terminal_read;
    stdin_kernel.write = terminal_write;
    stdin_kernel.open = terminal_open;
    stdin_kernel.close = terminal_close;
    stdin_kernel.inode_num = 0;
    stdin_kernel.file_pos = 0;
    stdin_kernel.flags = 1; // In-use
    kernel_file_array[STDIN_FD] = stdin_kernel;

    // Initialize kernel stdout file desctriptor
    file_desc_t stdout_kernel;
    memset(&stdout_kernel, 0x00, sizeof(file_desc_t));
    stdout_kernel.read = terminal_read;
    stdout_kernel.write = terminal_write;
    stdout_kernel.open = terminal_open;
    stdout_kernel.close = terminal_close;
    stdout_kernel.inode_num = 0;
    stdout_kernel.file_pos = 0;
    stdout_kernel.flags = 1; // In-use
    kernel_file_array[STDOUT_FD] = stdout_kernel;
}

/**
 *
 */
pcb_t* init_pcb(uint32_t pid) {
    pcb_t pcb;
    memset(&pcb, 0x00, sizeof(pcb_t));

    pcb.pid = pid;

    // Initialize stdin file desctriptor
    file_desc_t stdin;
    memset(&stdin, 0x00, sizeof(file_desc_t));
    stdin.read = terminal_read;
    stdin.write = terminal_write;
    stdin.open = terminal_open;
    stdin.close = terminal_close;
    stdin.inode_num = 0;
    stdin.file_pos = 0;
    stdin.flags = 1; // In-use
    pcb.file_array[STDIN_FD] = stdin;

    // Initialize kernel stdout file desctriptor
    file_desc_t stdout;
    memset(&stdout, 0x00, sizeof(file_desc_t));
    stdout.read = terminal_read;
    stdout.write = terminal_write;
    stdout.open = terminal_open;
    stdout.close = terminal_close;
    stdout.inode_num = 0;
    stdout.file_pos = 0;
    stdout.flags = 1; // In-use
    pcb.file_array[STDOUT_FD] = stdout;

    pcb.terminal_index = 0; //TODO: Actual index

    // Place into memory
    void* pcb_mem_location = (void*) ((8 * MB) - ((pid + 1) * (8 * KB)));
    memcpy(pcb_mem_location, &pcb, sizeof(pcb_t));

    return (pcb_t*) pcb_mem_location;
}

/**
 *
 */
pcb_t* get_pcb_ptr() {

    /*
     * We only want to return the pcb pointer if are running a process. If
     * this function is called by the kernel before starting the shell, it will
     * return NULL
     */
    int i;
    for(i = 1; i <= MAX_TASKS; i++) {
        if(pid_use_array[i] != 0) {
            register uint32_t esp asm ("esp");
            return (pcb_t*) (esp & 0xFFFFE000);
        }
    }
    return NULL;
}

/**
 *
 */
file_desc_t* get_file_array() {
    pcb_t* pcb = get_pcb_ptr();
    return (pcb == NULL) ? kernel_file_array : pcb->file_array;
}

/**
 *
 */
void task_switch(uint32_t new_pid) {
    log(DEBUG, "Switching to new task!", "task_switch");

    if(new_pid == 0) {
        log(ERROR, "Can't switch to kernel", "task_switch");
        return;
    }

    pcb_t* old_pcb = get_pcb_ptr();
    if(old_pcb == NULL) {
        log(ERROR, "Can't switch away from the kernel!", "task_switch");
        return;
    }

    if(old_pcb->pid == new_pid) {
        log(WARN, "Can't switch to the current active task", "task_switch");
        return;
    }

    // Write TSS with new process's kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((8 * MB) - ((new_pid) * (8 * KB)) - 4);

    // new_pcb is stored at the top of the new process's stack
    //pcb_t* new_pcb = (void*) tss.esp0;

    // Save esp/ebp in the PCB
    /*
    register uint32_t esp asm ("esp");
    old_pcb->old_esp = esp;
    register uint32_t ebp asm ("ebp");
    old_pcb->old_ebp = ebp;
    */

    // Save old ESP and EBP in registers
    //register uint32_t old_esp = new_pcb->old_esp;
    //register uint32_t old_ebp = new_pcb->old_ebp;

    // Restore new process's paging?
    restore_parent_paging(old_pcb->pid, new_pid);

    // Do some video memory bullshit

    // Put new process's shit in ESP/EBP
    //asm volatile ("movl %0, %%esp;"::"r"(old_esp));
    //asm volatile ("movl %0, %%ebp;"::"r"(old_ebp));

    return;
}
