#include "rtc.h"
#include "../lib.h"
#include "../interrupts/interrupts.h"

void init_rtc(void) {
  set_rtc_frequency(0x2F);

  // Enable interrupts
  outb(RTC_REGB, RTC_INDEX_PORT);
  uint8_t prev = inb(RTC_DATA_PORT);
  outb(RTC_REGB, RTC_INDEX_PORT);
  outb(prev | 0x40, RTC_DATA_PORT);
  enable_irq(RTC_IRQ); // Enable RTC
}

void set_rtc_frequency(unsigned char frequency) {
  // Set RTC interrupt frequency
  outb(RTC_REGA, RTC_INDEX_PORT);
  outb(frequency, RTC_DATA_PORT); // 0bx0101111 (DV = 2, RS = 15)
}
