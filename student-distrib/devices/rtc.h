
#ifndef _RTC_H
#define _RTC_H

uint32_t volatile tick_counter;

int32_t rtc_init();
int32_t rtc_set_frequency(int32_t rate);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);



// for old code
//#define RTC_REGA 0x8A
//#define RTC_REGB 0x8B
//
//void init_rtc(void);
//
//void set_rtc_frequency(unsigned char frequency);

#endif
