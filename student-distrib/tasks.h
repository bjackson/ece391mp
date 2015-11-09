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

// File descriptor table used by the kernel (will probably be moved later)
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

//
void init_kernel_file_array();

#endif // TASKS_H
