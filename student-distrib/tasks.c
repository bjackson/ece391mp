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

