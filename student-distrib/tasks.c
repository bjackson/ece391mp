/**
 * tasks.c
 *
 * vim:ts=4 expandtab
 */
#include "tasks.h"

// File descriptor table used by the kernel (will probably be moved later)
file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

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
void init_pcb(uint32_t pid) {
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
    void* pcb_mem_location = (void*) ((8 * MB) - (pid * (8 * KB)));
    memcpy(pcb_mem_location, &pcb, sizeof(pcb_t));
}

/**
 *
 */
pcb_t* get_pcb_ptr() {
    register uint32_t esp asm ("esp");
    return (pcb_t*) (esp & 0xFFFFE000);
}
