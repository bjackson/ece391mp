/**
 * filesys.c - <description here>
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
    file_desc_t file = kernel_file_array[fd];
    if(file.flags & 0x1) {
        int32_t bytes_read = read_data(file.inode_num, file.file_pos, buf, nbytes);
        file.file_pos += bytes_read;

        return bytes_read;//
    } else {
        printf("Filesys error: Specified file is closed");
        return -1;
    }
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
        printf("Filesys error: NULL dentry pointer\n");
        return -1;
    }

    uint8_t i;
    for(i = 0; i < fs_stats->num_dentries; i++) {
        dentry_t entry;
        read_dentry_by_index(i, &entry);

        //TODO: Only use 32 bytes, i.e. long filename should still match
        if(strncmp(entry.fname, (int8_t*) fname, FS_FNAME_LEN) == 0) {
            memcpy(dentry, &entry, sizeof(dentry_t));
            return 0;
        }
    }

    printf("Filesys error: dentry by name not found\n");
    return -1;
}

/**
 *
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if(index >= fs_stats->num_dentries) {
        printf("Filesys error: dentry index invalid");
        return -1;
    }

    dentry_t entry = boot_block->entries[index];
    memcpy(dentry, &entry, sizeof(dentry_t));
    return 0;
}

/**
 *
 */
//TODO: Use this function and use offset, buf, length
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    return -1;
}

/**
 *
 */
int32_t fs_read_dir() {
    printf("\nDirectory Contents:\n");
    uint8_t i;
    for(i = 0; i < fs_stats->num_dentries; i++) {
        dentry_t entry;
        read_dentry_by_index(i, &entry);

        //TODO: Don't need to not show RTC and dir
        if(entry.type == FS_TYPE_FILE) {
            inode_t* node = (inode_t*) (fs_inode_start_addr +
                    entry.inode_num * FS_BLOCK_SIZE);

            printf("name: %s, ", entry.fname);
            printf("len: %dB\n", node->length);
        }
    }

    return 0;
}

void fs_test_print_file(const uint8_t* name) {
    printf("\nFile: %s:\n", name);

    dentry_t entry;
    if(read_dentry_by_name(name, &entry) == -1) {
        printf("Filesys test error: Cant read dentry by name\n");
        return;
    }

    inode_t* node = (inode_t*) (fs_inode_start_addr +
            entry.inode_num * FS_BLOCK_SIZE);

    uint32_t bytes_left = node->length;

    while(bytes_left > 0) {
        uint32_t index = ((node->length - bytes_left) / FS_BLOCK_SIZE);

        void* data_block_ptr = (void*) (fs_data_start_addr +
                (node->blocks[index] * FS_BLOCK_SIZE));

        char file_buffer[FS_BLOCK_SIZE];
        memset(file_buffer, 0x00, FS_BLOCK_SIZE);

        if(bytes_left >= FS_BLOCK_SIZE) {
            memcpy(file_buffer, data_block_ptr, FS_BLOCK_SIZE);
            printf("%s", file_buffer);
            bytes_left -= FS_BLOCK_SIZE;
        } else {
            memcpy(file_buffer, data_block_ptr, bytes_left);
            printf("%s\n", file_buffer);
            break;
        }
    }
}

//TODO: Read teh whole thing, even if only printing out a few bytes
void fs_test_print_exec(const uint8_t* name) {
    printf("\nExec: %s:\n", name);

    dentry_t entry;
    if(read_dentry_by_name(name, &entry) == -1) {
        printf("Filesys test error: Cant read dentry by name\n");
        return;
    }

    inode_t* node = (inode_t*) (fs_inode_start_addr +
            entry.inode_num * FS_BLOCK_SIZE);

    void* data_block_ptr = (void*) (fs_data_start_addr +
            (node->blocks[0] * FS_BLOCK_SIZE));

    char file_buffer[16];
    memset(file_buffer, 0x00, 16);

    memcpy(file_buffer, data_block_ptr, 16);
    printf("%s\n", file_buffer);
}

void fs_test() {
    // Print out a complete file
    fs_test_print_file((uint8_t*) "frame0.txt");

    // Print out a very long file
    fs_test_print_file((uint8_t*) "verylargetxtwithverylongname.tx");

    // Print out the first few bytes of an executable
    fs_test_print_exec((uint8_t*) "cat");

    // List directory contents - include file lengths and some block nums
    fs_read_dir();

    // Disable interrupts and spin forever so the screen isn't cleared
    cli();
    while(1);
}

