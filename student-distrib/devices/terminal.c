#include "terminal.h"
#include "../lib.h"

uint32_t keyboard_buffer_index;

uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
uint8_t read_buffer[KEYBOARD_BUFFER_SIZE];

uint32_t readyToRead;

int32_t backspace(void) {
  keyboard_buffer_index--;
  keyboard_buffer[keyboard_buffer_index] = '\0';
  return 0;
}

int32_t terminal_open(const uint8_t* filename) {
  keyboard_buffer_index = 0;
  readyToRead = FALSE;
  return 0;
}

int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
  memset(read_buffer, '\0', 128); // Clear read buffer from previous calls
  uint32_t bufferIndex = 0;

  uint32_t spin = 0;
  while (readyToRead != TRUE) {
    spin++;
  }

  while (bufferIndex <= KEYBOARD_BUFFER_SIZE - 1) {
    read_buffer[bufferIndex] = keyboard_buffer[bufferIndex];
    if (keyboard_buffer[bufferIndex] == '\n') {
      bufferIndex++;
      break;
    }
    bufferIndex++;
  }
  keyboard_buffer_index = 0;
  readyToRead = 0;
  memset(keyboard_buffer, '\0', 128); // Clear keyboard buffer for new input
  return bufferIndex;
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
  printf((int8_t *)buf);
  return strlen(buf);
}

int32_t terminal_close(int32_t fd) {
  // Terminal should not be allowed to be closed.
  return -1;
}

int32_t terminal_clear(void) {
  clear();

  return 0;
}
