/**
 * tasks.c
 *
 * vim:ts=4 expandtab
 */
#include "tasks.h"

// File descriptor table used by the kernel (will probably be moved later)
file_desc_t kernel_file_array[FILE_ARRAY_SIZE];

// If PID in use, pid_use_array[pid] = 1, 0 otherwise
uint32_t pid_use_array[MAX_TASKS + 1] = {0};

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

/**
 *
 */
pcb_t* init_pcb(uint32_t pid) {
    pcb_t pcb;
    memset(&pcb, 0x00, sizeof(pcb_t));

    pcb.pid = pid;

    // Initialize stdin file desctriptor
    file_desc_t stdin;
    memset(&stdin, 0x00, sizeof(file_desc_t));
    stdin.read = terminal_read;
    stdin.write = terminal_write;
    stdin.open = terminal_open;
    stdin.close = terminal_close;
    stdin.inode_num = 0;
    stdin.file_pos = 0;
    stdin.flags = 1; // In-use
    pcb.file_array[STDIN_FD] = stdin;

    // Initialize kernel stdout file desctriptor
    file_desc_t stdout;
    memset(&stdout, 0x00, sizeof(file_desc_t));
    stdout.read = terminal_read;
    stdout.write = terminal_write;
    stdout.open = terminal_open;
    stdout.close = terminal_close;
    stdout.inode_num = 0;
    stdout.file_pos = 0;
    stdout.flags = 1; // In-use
    pcb.file_array[STDOUT_FD] = stdout;

    // Place into memory
    void* pcb_mem_location = (void*) ((8 * MB) - ((pid + 1) * (8 * KB)));
    memcpy(pcb_mem_location, &pcb, sizeof(pcb_t));

    return (pcb_t*) pcb_mem_location;
}

/**
 *
 */
pcb_t* get_pcb_ptr() {

    /*
     * We only want to return the pcb pointer if are running a process. If
     * this function is called by the kernel before starting the shell, it will
     * return NULL
     */
    int i;
    for(i = 1; i <= MAX_TASKS; i++) {
        if(pid_use_array[i] != 0) {
            register uint32_t esp asm ("esp");
            return (pcb_t*) (esp & 0xFFFFE000);
        }
    }
    return NULL;
}

pcb_t* get_pcb_ptr_for_pid(int32_t pid) {
  assert_do(pid < MAX_TASKS + 1, {
    return NULL;
  });

  assert_do(pid_use_array[pid] != 0, {
    return NULL;
  });

  pcb_t *start_addr = (pcb_t *)(((4 * MB) * (1 + pid)) & 0xFFFFE000);
  return start_addr;
}

int32_t get_number_of_running_tasks(void) {
  int number_of_running_tasks = 0;
  int i;
  for (i = 1; i < MAX_TASKS + 1; i++) {
    if (pid_use_array[i] != 0) {
      number_of_running_tasks++;
    }
  }

  return number_of_running_tasks;
}

/**
 *
 */
file_desc_t* get_file_array() {
    pcb_t* pcb = get_pcb_ptr();
    return (pcb == NULL) ? kernel_file_array : pcb->file_array;
}
