#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "types.h"
#include "tasks.h"

#define SCHEDULER_TASK_INTERVAL_LEN 50 // milliseconds


int32_t run_next_task(task_registers_t task_registers);

#endif
