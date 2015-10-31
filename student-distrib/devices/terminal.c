#include "terminal.h"
#include "../lib.h"

#define KEYBOARD_BUFFER_SIZE

uint8_t keyboard_buffer[128];

int32_t terminal_open(const uint8_t* filename) {
  return 0;
}

int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
  return 0;
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
  printf((int8_t *)buf);
  return strlen(buf);
}

int32_t terminal_close(int32_t fd) {
  return -1;
}
