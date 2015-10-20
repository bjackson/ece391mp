#include "x86_desc.h"
#include "lib.h"
#include "interrupts.h"

#define SYSTEM_CALL_IDT_LOC 0x80

void systemCallHandler();

// Initializes the IDT
// @return 0 on success
int initializeIDT() {

  // Initialize system call interrupt
  idt[SYSTEM_CALL_IDT_LOC].seg_selector = KERNEL_CS;
  idt[SYSTEM_CALL_IDT_LOC].reserved4 = 0;
  idt[SYSTEM_CALL_IDT_LOC].reserved3 = 0;
  idt[SYSTEM_CALL_IDT_LOC].reserved2 = 1;
  idt[SYSTEM_CALL_IDT_LOC].reserved1 = 1;
  idt[SYSTEM_CALL_IDT_LOC].size = 1;
  idt[SYSTEM_CALL_IDT_LOC].reserved0 = 0;
  idt[SYSTEM_CALL_IDT_LOC].dpl = 3;
  idt[SYSTEM_CALL_IDT_LOC].present = 1;

  SET_IDT_ENTRY(idt[SYSTEM_CALL_IDT_LOC], systemCallHandler);


  return 0;
}
