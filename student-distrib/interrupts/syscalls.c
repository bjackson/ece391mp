/**
 * syscalls.c
 *
 * vim:ts=4 expandtab
 */
#include "syscalls.h"

// Declared in tasks.c
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

/**
 *
 */
extern int32_t sys_halt(uint8_t status) {
    printf("Halt!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_execute(const uint8_t* command) {
    printf("Execute!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    printf("Read!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    printf("Write!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_open(const uint8_t* filename) {
    if(filename == NULL) {
        printf("open: NULL filename\n");
        return -1;
    }

    dentry_t dentry;
    if(read_dentry_by_name(filename, &dentry) == -1) {
        printf("open: Named file does not exist\n");
        return -1;
    }

    int i;
    for(i = 2; i < FILE_ARRAY_SIZE; i++) {
        file_desc_t file = kernel_file_array[i];

        // Check if file descriptor is unused
        if((file.flags & 0x1) == 0) {

            file.flags |= 0x1; // Mark as in-use
            file.inode_num = dentry.inode_num;

            if(dentry.type == FS_TYPE_RTC) {
                file.read = rtc_read;
                file.write = rtc_write;
                file.open = rtc_open;
                file.close = rtc_close;
            } else if(dentry.type == FS_TYPE_DIR) {
                file.file_pos = 0;
                file.read = fs_read;
                file.write = fs_write;
                file.open = fs_open;
                file.close = fs_close;
            } else if(dentry.type == FS_TYPE_FILE) {
                file.file_pos = 0;
                file.read = fs_read;
                file.write = fs_write;
                file.open = fs_open;
                file.close = fs_close;
            } else {
                memset(&file, 0x00, sizeof(file_desc_t));
                kernel_file_array[i] = file;
                printf("open: Invalid dentry type\n");
                return -1;
            }

            kernel_file_array[i] = file;

            // Pass-through to specific open() function
            if(file.open(filename) == -1) {
                printf("open: specific open() function failed\n");
                return -1;
            }

            // Return newly allocated file descriptor
            return i;
        }
    }

    printf("open: No remaining file descriptors\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_close(int32_t fd) {
    if(fd == STDIN_FD || fd == STDOUT_FD) {
        printf("close: Can't close stdin/stdout\n");
        return -1;
    }

    file_desc_t file = kernel_file_array[fd];
    if(file.flags & 0x1) {
        // Remove the file descriptor from the file array
        file_desc_t empty;
        memset(&empty, 0x00, sizeof(file_desc_t));
        kernel_file_array[fd] = empty;

        // Pass-through to specific close() function
        return file.close(fd);
    } else {
        printf("close: Invalid file descriptor\n");
        return -1;
    }
}

/**
 *
 */
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    printf("Get Args!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_vidmap(uint8_t** screen_start) {
    printf("Vidmap!\n");
    return -1;
}

/**
 *
 */
int32_t debug_do_call(int32_t number, int32_t arg1, int32_t arg2, int32_t arg3) {
    asm volatile (
            "pushl %%ebx;"
            "movl 12(%%esp), %%eax;"
            "movl 16(%%esp), %%ebx;"
            "movl 20(%%esp), %%ecx;"
            "movl 24(%%esp), %%edx;"
            "int $0x80;"
            "popl %%ebx;" : : );
    register int32_t retval asm("eax");
    return retval;
}
