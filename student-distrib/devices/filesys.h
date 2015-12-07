/**
 * filesys.h
 * vim:ts=4 expandtab
 */

#ifndef _FILESYS_H
#define _FILESYS_H

#include "../types.h"
#include "../lib.h"
#include "../tasks.h"
#include "../interrupts/syscalls.h"

#define FS_BLOCK_SIZE 4096
#define FS_FNAME_LEN  32

#define FS_TYPE_RTC  0
#define FS_TYPE_DIR  1
#define FS_TYPE_FILE 2

// Struct for directory entries
typedef struct {
    char fname[32];
    uint32_t type;
    uint32_t inode_num;
    uint8_t reserved[24];
} dentry_t;

// Struct for filesystem statistics
typedef struct {
    uint32_t num_dentries;
    uint32_t num_inodes;
    uint32_t num_datablocks;
    uint8_t reserved[52];
} fs_stats_t;

// Struct for inodes
typedef struct {
    uint32_t length;
    uint32_t blocks[(FS_BLOCK_SIZE / 4) - 1];
} inode_t;

// Struct for filesys boot block
typedef struct {
    fs_stats_t stats;
    dentry_t entries[63];
} boot_block_t;

// initialize the file system
void fs_init(uint32_t fs_start_addr);

// open
int32_t fs_open(const uint8_t* fname);

// close
int32_t fs_close(int32_t fd);

// read
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes);

// directory read
int32_t fs_dir_read(int32_t fd, void* buf, int32_t nbytes);

// write
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes);

// read dentry
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

// read dentry using the index
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

// read data
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// length
int32_t fs_len(int32_t fd);

// seek
int32_t fs_seek(int32_t fd, uint32_t pos);

// test file system
void fs_test();

#endif /* _FILESYS_H */
