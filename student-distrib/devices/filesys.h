/**
 * filesys.h - <description here>
 * vim:ts=4 expandtab
 */

#ifndef _FILESYS_H
#define _FILESYS_H

#include "../types.h"

#define FS_BLOCK_SIZE 4096

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

//
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

//
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

//
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif /* _FILESYS_H */
