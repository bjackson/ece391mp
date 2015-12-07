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

// Declared in syscalls.c
extern void* halt_ret_lbl asm("halt_ret_lbl");

// Declared in terminal.c
extern volatile uint32_t active_pids[NUM_TERMINALS];

/*
* void init_kernel_file_array()
*   Inputs:
*   Return Value: none
*   Function: begins filesystem processing by the kernel
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

/*
* pcb_t* init_pcb(uint32_t pid)
*   Inputs:
*   -pid = process id
*   Return Value: pointer to PCB data sctructure
*   Function: initializes PCB for the given process ID
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

    // Place into memory
    void* pcb_mem_location = (void*) ((8 * MB) - ((pid + 1) * (8 * KB)));
    memcpy(pcb_mem_location, &pcb, sizeof(pcb_t));

    return (pcb_t*) pcb_mem_location;
}

/*
* pcb_t* get_pcb_ptr()
*   Inputs:
*   Return Value: pointer to the PCB if a process is running
*   Function: grabs PCB
*/
pcb_t* get_pcb_ptr() {

    /*
     * We only want to return the pcb pointer if we are running a process. If
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
pcb_t* get_pcb_ptr_pid(uint32_t pid) {
    return (pcb_t*) ((8 * MB) - ((pid + 1) * (8 * KB)));
}

/*
* file_desc_t* get_file_array()
*   Inputs:
*   Return Value: file descriptor array pointer
*   Function: grabs the value of the file array for the given PCB
*/
file_desc_t* get_file_array() {
    pcb_t* pcb = get_pcb_ptr();
    return (pcb == NULL) ? kernel_file_array : pcb->file_array;
}

/*
* void task_switch(uint32_t new_pid)
*   Inputs:
*   -new_pid = process ID to switch to
*   Return Value: None
*   Function: handles the bulk of the task switching
*/
void task_switch(uint32_t new_pid) {
    cli(); // Begin critical section
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

    // Mark the new task as the active task of the current terminal
    active_pids[current_terminal] = new_pid;

    // Write TSS with new process's kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((8 * MB) - ((new_pid) * (8 * KB)) - 4);

    // Get the PCB for the new task
    pcb_t* new_pcb = (pcb_t*) ((8 * MB) - ((new_pid + 1) * (8 * KB)));

    // Save esp/ebp in the PCB, and mark that we came from this function
    register uint32_t esp asm ("esp");
    old_pcb->switch_esp = esp;
    register uint32_t ebp asm ("ebp");
    old_pcb->switch_ebp = ebp;
    old_pcb->from_task_switch = 1;

    // Restore new process's paging
    restore_parent_paging(old_pcb->pid, new_pid);

    /*
     * Remap old tasks's VIDEO memory to backing store and new tasks's video
     * memory to physical VIDEO addr
     */
    remap_video_memory(old_pcb->pid, new_pid);

    // Ensure the screen displays properly based on the active task
    switch_active_terminal_screen((old_pcb == NULL) ? KERNEL_PID : old_pcb->pid, new_pid);

    /*
     * If this kernel stack didn't leave off at task_switch code, we need to head
     * back to sys_execute as that's where it left off. It should only have to return
     * there once. This ensures the stack is dealt with properly.
     */
    if(!new_pcb->from_task_switch) { // Stack for parent process was stored in the old pcb
        // Restore the stack of the parent process
        asm volatile ("movl %0, %%esp;"::"r"(old_pcb->parent_esp));
        asm volatile ("movl %0, %%ebp;"::"r"(old_pcb->parent_ebp));
        asm volatile ("jmp halt_ret_lbl;");
    }

    log(DEBUG, "Returning from task_switch normally", "task_switch");

    // Restore the stack of the new process
    asm volatile ("movl %0, %%esp;"::"r"(new_pcb->switch_esp));
    asm volatile ("movl %0, %%ebp;"::"r"(new_pcb->switch_ebp));

    sti(); // End critical section
    return;
}
