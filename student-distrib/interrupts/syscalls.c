/**
 * syscalls.c
 *
 * vim:ts=4 expandtab
 */
#include "syscalls.h"

/*
 * This label needs to be global so that we can jump to it from other files,
 * namely from task_switch in tasks.c
 */
asm(".globl halt_ret_lbl");

// Declared in tasks.c
extern file_desc_t kernel_file_array[FILE_ARRAY_SIZE];
extern uint32_t pid_use_array[MAX_TASKS + 1];

// Declared in terminal.c
extern volatile uint32_t shell_pids[NUM_TERMINALS];
extern volatile uint32_t active_pids[NUM_TERMINALS];
extern volatile uint32_t current_terminal;

/*
 * sys_halt(uint8_t status)
 * Decsription: Halt the system shells
 * Inputs: status - ignored
 * Outputs: -1 on failure, DEADBEEF on success
 */
int32_t sys_halt(uint8_t status) {
    pcb_t* pcb_ptr = get_pcb_ptr();
    if(pcb_ptr == NULL) {
        log(WARN, "Can't halt the kernel!", "halt");
        return -1;
    }
    pcb_t pcb = *pcb_ptr;

    // Clear pcb structure
    memset(get_pcb_ptr(), 0x00, sizeof(pcb_t));

    // Free up PID for future use
    pid_use_array[pcb.pid] = 0;

    // Check to see if we are halting the base shell for a terminal. If so, execute another
    int i;
    for(i = 0; i < NUM_TERMINALS; i++) {
        if(active_pids[i] == pcb.pid) {
            active_pids[i] = pcb.parent_pid;
        }

        if(shell_pids[i] == pcb.pid) {
            log(DEBUG, "Exiting base terminal. Executing another", "halt");
            shell_pids[i] = 0;
            do_execute((uint8_t*) "shell");
        }
    }

    // Write TSS with parent process's kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((8 * MB) - ((pcb.parent_pid) * (8 * KB)) - 4);

    // Save old ESP and EBP in registers
    register uint32_t parent_esp = pcb.parent_esp;
    register uint32_t parent_ebp = pcb.parent_ebp;

    // Restore parent's paging
    restore_parent_paging(pcb.pid, pcb.parent_pid);

    /*
     * Remap old tasks's VIDEO memory to backing store and new tasks's video
     * memory to physical VIDEO addr
     */
    remap_video_memory(pcb.pid, pcb.parent_pid);

    // Restore parent's ESP/EBP
    asm volatile ("movl %0, %%esp;"::"r"(parent_esp));
    asm volatile ("movl %0, %%ebp;"::"r"(parent_ebp));
    asm volatile ("jmp halt_ret_lbl;");

    // This function should never return to the caller
    return 0xDEADBEEF;
}

/*
 * sys_execute(const uint8_t* command)
 * Decsription: execute commands
 * Inputs: command - the command to execute
 * Outputs: -1 on failure, 0 on success
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
        log(WARN, "Filename invalid", "execute");
        return -1;
    }

    // Read the executable file header
    uint8_t header[EXE_HEADER_LEN];
    memset(header, 0x00, EXE_HEADER_LEN);
    if(sys_read(fd, header, EXE_HEADER_LEN) == -1) {
        log(WARN, "Can't read executable header", "execute");
        sys_close(fd);
        return -1;
    }

    // Seek to beginning of executable for program loader
    if(fs_seek(fd, 0) == -1) {
        log(WARN, "Couldn't seek to beginning of executable", "execute");
        sys_close(fd);
        return -1;
    }

    // Check for presence of magic number in header
    if(((uint32_t*) header)[EXE_HEADER_MAGICNUM_IDX] != EXE_HEADER_MAGIC) {
        log(WARN, "Magic number not present", "execute");
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
        log(WARN, "Reached maximum number of tasks", "execute");
        sys_close(fd);
        return -1;
    }

    // Mark the task we are executing as the active task of the current terminal
    active_pids[current_terminal] = new_pid;

    // Fetch old PCB structure (or NULL if we're running pre-task kernel)
    pcb_t* old_pcb = get_pcb_ptr();

    // Set up paging structures for new process
    init_task_paging(new_pid);

    /*
     * Remap old tasks's VIDEO memory to backing store and new tasks's video
     * memory to physical VIDEO addr
     */
    remap_video_memory((old_pcb == NULL) ? KERNEL_PID : old_pcb->pid, new_pid);

    // Check if we are executing a new base shell for a terminal
    if(strncmp(((int8_t*) executable_fname), "shell", 5) == 0) {
        if(shell_pids[current_terminal] == 0) {
            shell_pids[current_terminal] = new_pid;
            clear();
        }
    }

    // Load program image into memory from the file system
    void* program_image_mem = (void*) 0x08048000;
    if(sys_read(fd, program_image_mem, fs_len(fd)) == -1) {
        log(WARN, "Program loader read failed", "execute");
        sys_close(fd);
        restore_parent_paging(new_pid, (old_pcb == NULL) ? KERNEL_PID : old_pcb->pid);
        return -1;
    }

    // Close executable file, as it is now in memory
    if(sys_close(fd) == -1) {
        log(WARN, "Couldn't close executable file", "execute");
        restore_parent_paging(new_pid, (old_pcb == NULL) ? KERNEL_PID : old_pcb->pid);
        return -1;
    }

    // Set up process control block
    pcb_t* new_pcb = init_pcb(new_pid);
    new_pcb->terminal_index = current_terminal;

    // Mark PID as used
    pid_use_array[new_pid] = 1;

    // Put arguments in task's PCB
    memcpy(new_pcb->args, task_args, MAX_ARGS_LENGTH);

    // Initiate Context Switch

    // Write TSS with new process's kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = ((8 * MB) - ((new_pid) * (8 * KB)) - 4);

    // Save parent PID in the PCB. Should be KERNEL_PID for new base shells
    if(shell_pids[current_terminal] == new_pid) {
        new_pcb->parent_pid = KERNEL_PID;
    } else {
        new_pcb->parent_pid = (old_pcb == NULL) ? KERNEL_PID : old_pcb->pid;
    }

    // Save esp/ebp in the PCB
    register uint32_t esp asm ("esp");
    new_pcb->parent_esp = esp;
    register uint32_t ebp asm ("ebp");
    new_pcb->parent_ebp = ebp;

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

/*
 * sys_read(int32_t fd, void* buf, int32_t nbytes)
 * Decsription: system read
 * Inputs: fd - file descriptor, buf - buffer, nbytes - max size to read
 * Outputs: -1 on failure, number of read bytes on success
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    if(fd < 0 || fd >= FILE_ARRAY_SIZE) {
        log(WARN, "fd out of range", "read");
        return -1;
    }

    file_desc_t file = get_file_array()[fd];
    if(!(file.flags & 0x1)) {
        log(WARN, "Invalid file descriptor", "read");
        return -1;
    }

    sti(); // Enable further interrupts
    return file.read(fd, buf, nbytes);
}

/*
 * sys_write(int32_t fd, void* buf, int32_t nbytes)
 * Decsription: system write
 * Inputs: fd - file descriptor, buf - buffer, nbytes - max size to read
 * Outputs: -1 on failure, written file
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    if(fd < 0 || fd >= FILE_ARRAY_SIZE) {
        log(WARN, "fd out of range", "write");
        return -1;
    }

    file_desc_t file = get_file_array()[fd];
    if(!(file.flags & 0x1)) {
        log(WARN, "Invalid file descriptor", "write");
        return -1;
    }

    return file.write(fd, buf, nbytes);
}

/*
 * sys_open(const uint8_t* filename)
 * Decsription: system open
 * Inputs: filename - file to open
 * Outputs: -1 on failure, 1 on success
 */
int32_t sys_open(const uint8_t* filename) {
    if(filename == NULL) {
        log(WARN, "NULL filename", "open");
        return -1;
    }

    dentry_t dentry;
    if(read_dentry_by_name(filename, &dentry) == -1) {
        log(WARN, "Named file does not exist", "open");
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
                log(ERROR, "Invalid dentry type", "open");
                return -1;
            }

            file_array[i] = file;

            // Pass-through to specific open() function
            if(file.open(filename) == -1) {
                log(WARN, "specific open() function failed", "open");
                return -1;
            }

            // Return newly allocated file descriptor
            return i;
        }
    }

    log(WARN, "No remaining file descriptors", "open");
    return -1;
}

/*
 * sys_close(int32_t fd)
 * Decsription: system open
 * Inputs: fd - file descriptor
 * Outputs: -1 on failure, file close
 */
int32_t sys_close(int32_t fd) {
    if(fd < 0 || fd >= FILE_ARRAY_SIZE) {
        log(WARN, "fd out of range", "close");
        return -1;
    }

    if(fd == STDIN_FD || fd == STDOUT_FD) {
        log(WARN, "Can't close stdin/stdout", "close");
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
        log(WARN, "Invalid file descriptor", "close");
        return -1;
    }
}

/*
 * sys_getargs(uint8_t* buf, int32_t nbytes)
 * Decsription: get argumants from buffer
 * Inputs: buf - buffer, nbytes - max size to read
 * Outputs: -1 on failure, 0 on success
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    if(buf == NULL) {
        log(WARN, "User-supplied buffer is null", "getargs");
        return -1;
    }

    pcb_t *pcb = get_pcb_ptr();

    uint32_t args_size = strlen((int8_t*) (pcb->args));
    if(args_size + 1 > nbytes) { // Add one to account for NULL-termination
        log(WARN, "user-supplied buffer is too small", "getargs");
        return -1;
    }

    memset(buf, 0x00, nbytes);
    memcpy(buf, pcb->args, (nbytes > MAX_ARGS_LENGTH) ? MAX_ARGS_LENGTH : nbytes);

    return 0;
}

/*
 * sys_vidmap(uint8_t** screen_start)
 * Decsription: map video memory
 * Inputs: screen_start - starting address of the screen
 * Outputs: -1 on failure, 0 on success
 */
int32_t sys_vidmap(uint8_t** screen_start) {
    if(screen_start == NULL) {
        log(WARN, "NULL screen_start addr", "vidmap");
        return -1;
    }

    if(((uint32_t) screen_start) < (128 * MB) ||
            ((uint32_t) screen_start) >= (132 * MB)) {
        log(WARN, "screen_start addr out of range", "vidmap");
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

/*
 * sys_set_handler(int32_t signum, void* handler_address)
 * Decsription: set handler
 * Inputs: signum - ignored, handler_address - ignored
 * Outputs: -1 always
 */
int32_t sys_set_handler(int32_t signum, void* handler_address) {
  return -1;
}


/*
 * sys_sigreturn(void)
 * Decsription: sigreturn
 * Inputs: void - ignored
 * Outputs: -1 always
 */
int32_t sys_sigreturn(void) {
  return -1;
}

/*
 * do_syscall(int32_t number, int32_t arg1, int32_t arg2, int32_t arg3)
 * Decsription: sigreturn
 * Inputs: void - ignored
 * Outputs: contents of eax
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
