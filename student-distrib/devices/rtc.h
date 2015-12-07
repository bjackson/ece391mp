
#ifndef _RTC_H
#define _RTC_H

#include "../types.h"
#include "../lib.h"

#define RTC_REGA 0x8A
#define RTC_REGB 0x8B

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71

#define MIN_FREQ 2
#define MAX_FREQ 1024

volatile uint32_t tick_counter;

// initializes the RTC
int32_t rtc_init();

// resets the frequency to the default
int32_t rtc_set_frequency(int32_t freq);

// dodes nothing
int32_t rtc_open(const uint8_t* filename);

// 
int32_t rtc_close(int32_t fd);

// read the frequency
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

// write the new frequency
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
