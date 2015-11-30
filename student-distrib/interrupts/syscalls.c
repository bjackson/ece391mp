/**
 * syscalls.c
 *
 * vim:ts=4 expandtab
 */
#include "syscalls.h"

// Declared in tasks.c
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];
extern uint32_t pid_use_array[MAX_TASKS + 1];

/**
 *
 */
int32_t sys_halt(uint8_t status) {
    pcb_t* pcb_ptr = get_pcb_ptr();
    if(pcb_ptr == NULL) {
        printf("halt: Can't halt the kernel!\n");
        return -1;
    }
    pcb_t pcb = *pcb_ptr;

    // Clear pcb structure
    memset(get_pcb_ptr(), 0x00, sizeof(pcb_t));

    // Free up PID for future use
    pid_use_array[pcb.pid] = 0;

    // Write TSS with parent process's kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((8 * MB) - ((pcb.parent_pid) * (8 * KB)) - 4);

    // Save old ESP and EBP in registers
    register uint32_t old_esp = pcb.old_esp;
    register uint32_t old_ebp = pcb.old_ebp;

    // Restore parent's paging
    restore_parent_paging(pcb.pid, pcb.parent_pid);

    // Restore parent's ESP/EBP
    asm volatile ("movl %0, %%esp;"::"r"(old_esp));
    asm volatile ("movl %0, %%ebp;"::"r"(old_ebp));
    asm volatile ("jmp halt_ret_lbl;");

    // This function should never return to the caller
    return 0xDEADBEEF;
}

/**
 *
 */
int32_t sys_execute(const uint8_t* command) {
    // Copy command until first space as executable
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

    uint8_t task_args[MAX_ARGS_LENGTH];
    memset(task_args, 0x00, MAX_ARGS_LENGTH);
    // Copy args into task_args
    int ia = i;
    if (command[ia] == ' ') {
      ia++;
      while (ia < MAX_ARGS_LENGTH && ia < strlen((int8_t*)command) && command[ia] != 0x00) {
        task_args[ia - i - 1] = command[ia];
        ia++;
      }
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
    if(sys_read(fd, header, EXE_HEADER_LEN) == -1) {
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

    // Fetch old PCB structure (or NULL if we're running pre-task kernel)
    pcb_t* old_pcb = get_pcb_ptr();

    // Load program image into memory from the file system
    void* program_image_mem = (void*) 0x08048000;
    if(sys_read(fd, program_image_mem, fs_len(fd)) == -1) {
        printf("execute: Program loader read failed\n");
        sys_close(fd);
        restore_parent_paging(new_pid, (old_pcb == NULL) ? KERNEL_PID : old_pcb->pid);
        return -1;
    }

    // Close executable file, as it is now in memory
    if(sys_close(fd) == -1) {
        printf("execute: Couldn't close executable file\n");
        restore_parent_paging(new_pid, (old_pcb == NULL) ? KERNEL_PID : old_pcb->pid);
        return -1;
    }

    // Set up process control block
    pcb_t* new_pcb = init_pcb(new_pid);

    // Mark PID as used
    pid_use_array[new_pid] = 1;

    // Put arguments in task's PCB
    memcpy(new_pcb->args, task_args, MAX_ARGS_LENGTH);

    // Initiate Context Switch

    // Write TSS with new process's kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((8 * MB) - ((new_pid) * (8 * KB)) - 4);

    // Save esp/ebp and the parent PID in the PCB
    new_pcb->parent_pid = (old_pcb == NULL) ? KERNEL_PID : old_pcb->pid;
    register uint32_t esp asm ("esp");
    new_pcb->old_esp = esp;
    register uint32_t ebp asm ("ebp");
    new_pcb->old_ebp = ebp;

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

    // Back from halt()!
    asm volatile("halt_ret_lbl:");
    sti();
    return 0; //TODO: status
}

/**
 *
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    file_desc_t file = get_file_array()[fd];
    sti(); // Enable further interrupts
    return file.read(fd, buf, nbytes);
}

/**
 *
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    file_desc_t file = get_file_array()[fd];
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

    file_desc_t* file_array = get_file_array();

    int i;
    for(i = 2; i < FILE_ARRAY_SIZE; i++) {
        file_desc_t file = file_array[i];

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
                file_array[i] = file;
                printf("open: Invalid dentry type\n");
                return -1;
            }

            file_array[i] = file;

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

    file_desc_t* file_array = get_file_array();

    file_desc_t file = file_array[fd];
    if(file.flags & 0x1) {
        // Remove the file descriptor from the file array
        file_desc_t empty;
        memset(&empty, 0x00, sizeof(file_desc_t));
        file_array[fd] = empty;

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
    if(buf == NULL) {
        printf("getargs: User-supplied buffer is null\n");
        return -1;
    }

    pcb_t *pcb = get_pcb_ptr();

    uint32_t args_size = strlen((int8_t*) (pcb->args));
    if(args_size + 1 > nbytes) { // Add one to account for NULL-termination
        printf("getargs: user-supplied buffer is too small. args size: (%d), buffer size: (%d)\n", args_size, nbytes);
        return -1;
    }

    memset(buf, 0x00, nbytes);
    memcpy(buf, pcb->args, (nbytes > MAX_ARGS_LENGTH) ? MAX_ARGS_LENGTH : nbytes);

    return 0;
}

/**
 *
 */
int32_t sys_vidmap(uint8_t** screen_start) {
    if(screen_start == NULL) {
        printf("vidmap: NULL screen_start addr\n");
        return -1;
    }

    if(((uint32_t) screen_start) < (128 * MB) ||
            ((uint32_t) screen_start) >= (132 * MB)) {
        printf("vidmap: screen_start addr out of range\n");
        return -1;
    }

    // Check to see if vidmap was called from the kernel
    if(get_pcb_ptr() == NULL) {
        *screen_start = (void*) VIDEO;
        return 0;
    }

    // Map phys addr VIDEO to virt addr 1GB
    mmap(((void*) VIDEO), ((void*) GB), ACCESS_ALL);
    *screen_start = (void*) GB;
    return 0;
}

/**
 *
 */
int32_t do_syscall(int32_t number, int32_t arg1, int32_t arg2, int32_t arg3) {
    asm volatile ("pushl %%ebx;\
                   movl %0, %%eax;\
                   movl %1, %%ebx;\
                   movl %2, %%ecx;\
                   movl %3, %%edx;\
                   int $0x80;\
                   popl %%ebx;" :: "g"(number), "g"(arg1), "g"(arg2), "g"(arg3));
    register int32_t retval asm("eax");
    return retval;
}

// Execute a command
// @param command the command to execute
// @return status code
/**
 *
 */
int32_t do_execute(uint8_t *command) {
  return do_syscall(SYSCALL_EXECUTE_NUM, (uint32_t) command, 0, 0);
}
