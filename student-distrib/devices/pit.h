#ifndef _PIT_H
#define _PIT_H

#include "../types.h"

#define PIT_IO_CHAN0  0x40
#define PIT_IO_CHAN1  0x41
#define PIT_IO_CHAN2  0x42
#define PIT_IO_CMD    0x43

#define PIT_CMD_MODE3 0x36

#define PIT_FREQUENCY 1193180 // Hz

int32_t set_pit_frequency(uint32_t frequency);



#endif
