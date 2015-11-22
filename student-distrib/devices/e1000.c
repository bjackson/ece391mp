#include "e1000.h"

int32_t e1000_init() {
  e1000_mmio = (uint32_t *)E1000_BASE;

  assert(e1000_mmio[E1000_STATUS] == E1000_STATUS_UNINIT);

  return 0;
}
