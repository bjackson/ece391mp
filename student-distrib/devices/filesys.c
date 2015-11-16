/**
 * filesys.c
 * vim:ts=4 expandtab
 */

#include "filesys.h"

static uint32_t fs_start_addr;
static uint32_t fs_inode_start_addr;
static uint32_t fs_data_start_addr;
static boot_block_t* boot_block;
static fs_stats_t* fs_stats;

// Declared in tasks.c
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

/**
 *
 */
void fs_init(uint32_t fs_start_addr_p) {
    fs_start_addr = fs_start_addr_p;

    boot_block = (boot_block_t*) fs_start_addr;
    fs_stats = &(boot_block->stats);

    fs_inode_start_addr = fs_start_addr + FS_BLOCK_SIZE;
    fs_data_start_addr = fs_inode_start_addr + (fs_stats->num_inodes * FS_BLOCK_SIZE);
}

/**
 *
 */
int32_t fs_open(const uint8_t* fname) {
    /**
     * File descriptors are set up in the generic open system call, so we have
     * nothing to do here (as of yet).
     */
    return 0;
}

/**
 *
 */
int32_t fs_close(int32_t fd) {
    /**
     * File descriptors are torn down in the generic close system call, so we
     * have nothing to do here (as of yet).
     */
    return 0;
}

/**
 *
 */
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes) {
    file_desc_t* file = &(get_file_array()[fd]);

    if((file->flags & 0x1) == 0) {
        printf("fs_read: Specified file is closed\n");
        return -1;
    }

    uint32_t bytes_read = read_data(file->inode_num, file->file_pos, buf, nbytes);
    file->file_pos += bytes_read;
    return bytes_read;
}

/**
 *
 */
int32_t fs_dir_read(int32_t fd, void* buf, int32_t nbytes) {
    file_desc_t* file = &(get_file_array()[fd]);

    if((file->flags & 0x1) == 0) {
        printf("fs_dir_read: Specified file is closed\n");
        return -1;
    }

    // If we have read all directory entries, return 0 indefinitely
    if(file->file_pos >= fs_stats->num_dentries) {
        return 0;
    }

    dentry_t entry;
    memset(&entry, 0x00, sizeof(dentry_t));
    read_dentry_by_index(file->file_pos, &entry);

    int32_t bytes_to_copy = (nbytes > FS_FNAME_LEN) ? FS_FNAME_LEN : nbytes;
    memcpy(buf, entry.fname, bytes_to_copy);
    file->file_pos++;
    return bytes_to_copy;
}

/**
 *
 */
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes) {
    // Read-only filesystem
    return -1;
}

/**
 *
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    if(dentry == NULL) {
        printf("read_dentry_by_name: NULL dentry pointer\n");
        return -1;
    }

    uint8_t i;
    for(i = 0; i < fs_stats->num_dentries; i++) {
        dentry_t entry;
        read_dentry_by_index(i, &entry);

        // Subtract 1 from FS_FNAME_LEN for null-terminating byte
        if(strncmp(entry.fname, (int8_t*) fname, FS_FNAME_LEN - 1) == 0) {
            memcpy(dentry, &entry, sizeof(dentry_t));
            return 0;
        }
    }

    printf("read_dentry_by_name: Not found\n");
    return -1;
}

/**
 *
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if(index >= fs_stats->num_dentries) {
        printf("read_dentry_by_index: Index invalid\n");
        return -1;
    }

    dentry_t entry = boot_block->entries[index];
    memcpy(dentry, &entry, sizeof(dentry_t));
    return 0;
}

/**
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    inode_t* node = (inode_t*) (fs_inode_start_addr + inode * FS_BLOCK_SIZE);

    // Return 0 if we've reached the end of the file
    if(offset >= node->length) {
        return 0;
    }

    uint32_t bytes_left = node->length - offset;
    bytes_left = bytes_left > length ? length : bytes_left;

    uint32_t bytes_read = 0;
    while(bytes_read < bytes_left) {
        uint32_t block_index = ((offset + bytes_read) / FS_BLOCK_SIZE);
        uint8_t* data_block_ptr = (uint8_t*) (fs_data_start_addr +
                (node->blocks[block_index] * FS_BLOCK_SIZE));
        buf[bytes_read] = data_block_ptr[(offset + bytes_read) % FS_BLOCK_SIZE];
        bytes_read++;
    }

    return bytes_read;
}

/**
 *
 */
int32_t fs_len(int32_t fd) {
    file_desc_t* file = &(get_file_array()[fd]);

    if((file->flags & 0x1) == 0) {
        printf("fs_len: Specified file is closed\n");
        return -1;
    }

    inode_t* node = (inode_t*) (fs_inode_start_addr + file->inode_num * FS_BLOCK_SIZE);
    return node->length;
}

/**
 *
 */
int32_t fs_seek(int32_t fd, uint32_t pos) {
    file_desc_t* file = &(get_file_array()[fd]);

    if((file->flags & 0x1) == 0) {
        printf("fs_seek: Specified file is closed\n");
        return -1;
    }

    file->file_pos = pos;
    return 0;
}

void fs_test() {
    uint8_t buffer[40];
    memset(buffer, 0x00, sizeof(buffer));

    // Open a file and read from it
    int32_t arg = (int32_t) (char*) "frame0.txt";
    int32_t fd = debug_do_call(5, arg, 0, 0); // open("frame0.txt");
    if(fd != -1) {
        while(fs_read(fd, buffer, sizeof(buffer) - 1) > 0) {
            printf("%s", buffer);
            memset(buffer, 0x00, sizeof(buffer) - 1);
        }
    }
    debug_do_call(6, fd, 0, 0); // close(fd);

    // Open a directory and read from it
    arg = (int32_t) (char*) ".";
    fd = debug_do_call(5, arg, 0, 0); // open(".");
    if(fd != -1) {
        while(fs_dir_read(fd, buffer, sizeof(buffer) - 1) > 0) {
            printf("%s\n", buffer);
            memset(buffer, 0x00, sizeof(buffer) - 1);
        }
    }
    debug_do_call(6, fd, 0, 0); // close(fd);

    // Disable interrupts and spin forever so the screen isn't cleared
    cli();
    while(1);
}

