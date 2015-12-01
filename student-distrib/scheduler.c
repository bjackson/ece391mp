#include "scheduler.h"

uint32_t current_task_idx = 0;

int32_t run_next_task(task_registers_t task_registers) {
  cli();

  pcb_t *current_task_pcb = get_pcb_ptr();
  //get_pcb_ptr_for_pid(current_task_idx);

  // uint32_t ebp, esp;
  // asm volatile (
  //   "movl %%ebp, %0 \n"
  //   "movl %%esp, %1 \n"
  //   : "=r" (ebp), "=r" (esp)
  //   :
  //   : "memory"
  // );

  register uint32_t esp asm ("esp");
  current_task_pcb->old_esp = esp;
  register uint32_t ebp asm ("ebp");
  current_task_pcb->old_ebp = ebp;

  // current_task_pcb->esp = esp;
  // current_task_pcb->ebp = ebp;

  int32_t number_of_running_tasks = get_number_of_running_tasks();

  if (number_of_running_tasks == 0) {
    return 0;
  }

  current_task_idx = (current_task_idx  + 1) % MAX_TASKS;

  // Make sure it's not the kernel task PID.
  current_task_idx = current_task_idx == 0 ? 1 : current_task_idx;

  // Move to the next task
  while (pid_use_array[current_task_idx] == 0) {
    current_task_idx = (current_task_idx  + 1) % MAX_TASKS;
    current_task_idx = current_task_idx == 0 ? 1 : current_task_idx;
  }

  pcb_t *next_task_pcb = get_pcb_ptr_for_pid(current_task_idx);



  sti();
  return 0;
}
