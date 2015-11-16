/**
 * syscalls.c
 *
 * vim:ts=4 expandtab
 */
#include "syscalls.h"

// Declared in tasks.c
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

// If PID in use, pid_use_array[pid] = 1, 0 otherwise
static uint32_t pid_use_array[MAX_TASKS + 1] = {0};

/**
 *
 */
int32_t sys_halt(uint8_t status) {
    printf("Halt!\n");
    return -1;
}

/**
 *
 */
int32_t sys_execute(const uint8_t* command) {
    // Copy command until first space as executable
    //TODO: parse args for getargs syscall
    uint8_t executable_fname[FS_FNAME_LEN];
    memset(executable_fname, 0x00, FS_FNAME_LEN);
    int i = 0;
    while(1) {
        if(command[i] == 0x00 || command[i] == ' ') {
            break;
        }
        executable_fname[i] = command[i];
        i++;
    }

    // Open the executable file
    int fd = sys_open(executable_fname);
    if(fd == -1) {
        printf("execute: Filename invalid\n");
        return -1;
    }

    // Read the executable file header
    uint8_t header[EXE_HEADER_LEN];
    memset(header, 0x00, EXE_HEADER_LEN);
    //if(sys_read(fd, header, EXE_HEADER_LEN) == -1) {
    if(fs_read(fd, header, EXE_HEADER_LEN) == -1) {
        printf("execute: Can't read executable header\n");
        sys_close(fd);
        return -1;
    }

    // Seek to beginning of executable for program loader
    if(fs_seek(fd, 0) == -1) {
        printf("execute: Couldn't seek to beginning of executable\n");
        sys_close(fd);
        return -1;
    }

    // Check for presence of magic number in header
    if(((uint32_t*) header)[EXE_HEADER_MAGICNUM_IDX] != EXE_HEADER_MAGIC) {
        printf("execute: Magic number not present\n");
        sys_close(fd);
        return -1;
    }

    // Get code entry point from header
    uint32_t entry_point = ((uint32_t*) header)[EXE_HEADER_ENTRY_IDX];

    cli(); // Begin critical section

    // Search for available PID
    int new_pid;
    for(new_pid = 1; new_pid <= MAX_TASKS; new_pid++) {
        if(pid_use_array[new_pid] == 0) {
            break;
        }
    }
    if(new_pid == MAX_TASKS + 1) {
        printf("execute: Reached maximum number of tasks\n");
        sys_close(fd);
        return -1;
    }

    // Set up paging structures for new process
    init_task_paging(new_pid);

    // Load program image into memory from the file system
    void* program_image_mem = (void*) 0x08048000;
    //if(sys_read(fd, program_image_mem, fs_len(fd)) == -1) {
    if(fs_read(fd, program_image_mem, fs_len(fd)) == -1) {
        printf("execute: Program loader read failed\n");
        sys_close(fd);
        //TODO: Restore paging
        return -1;
    }

    // Set up process control block
    init_pcb(new_pid);


    // Initiate Context Switch

    // Write TSS with new process's kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((8 * MB) - ((new_pid - 1) * (8 * KB)) - 4);

    //TODO: Save current esp/ebp or anything else in PCB

    // Load USER_DS into stack segment selectors
    asm volatile ("movw %w0, %%ax;"::"r"(USER_DS));
    asm volatile ("movw %%ax, %%ds;\
                   movw %%ax, %%es;\
                   movw %%ax, %%fs;\
                   movw %%ax, %%gs;":);

    // Push artificial IRET context on stack
    asm volatile ("pushl %0;"::"r"(USER_DS)); // Stack segment selector
    asm volatile ("pushl %0;"::"r"((132 * MB) - 4)); // Stack pointer
    asm volatile ("pushfl;\
                   popl %%ecx;\
                   orl  $0x200, %%ecx;\
                   pushl %%ecx;":); // EFLAGS (with IF set)
    asm volatile ("pushl %0;"::"r"(USER_CS)); // Code segment selector
    asm volatile ("pushl %0;"::"r"(entry_point)); // EIP

    // IRET - Going to user mode!
    asm volatile("iret;");

    //TODO: Put label here for sys_halt
    //TODO: Sti()?

    //sys_close(fd);
    return -1;
}

/**
 *
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    sti(); // Enable further interrupts
    file_desc_t file = get_pcb_ptr()->file_array[fd];
    return file.read(fd, buf, nbytes);
}

/**
 *
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    file_desc_t file = get_pcb_ptr()->file_array[fd];
    return file.write(fd, buf, nbytes);
}

/**
 *
 */
int32_t sys_open(const uint8_t* filename) {
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
                file.read = fs_dir_read;
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
int32_t sys_close(int32_t fd) {
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
int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    printf("Get Args!\n");
    return -1;
}

/**
 *
 */
int32_t sys_vidmap(uint8_t** screen_start) {
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
