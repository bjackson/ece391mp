
#ifndef _RTC_H
#define _RTC_H

#define RTC_REGA 0x8A
#define RTC_REGB 0x8B

void init_rtc(void);

void set_rtc_frequency(unsigned char frequency);

#endif
