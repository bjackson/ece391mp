#include "pit.h"
#include "../lib.h"

int32_t set_pit_frequency(uint32_t frequency) {
  uint32_t divisor = PIT_FREQUENCY / frequency;

  outb(PIT_CMD_MODE3, PIT_IO_CMD);
  outb(divisor & 0xff, PIT_IO_CHAN0);
  outb(divisor >> 8, PIT_IO_CHAN0);

  return 0;
}
