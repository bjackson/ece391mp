/**
 * syscalls.h
 *
 * vim:ts=4 expandtab
 */
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../types.h"
#include "../lib.h"
#include "../devices/filesys.h"
#include "../tasks.h"
#include "../devices/rtc.h"
#include "../paging.h"
#include "../x86_desc.h"
#include "../devices/e1000.h"
#include "../log.h"

#define EXE_HEADER_LEN            40
#define EXE_HEADER_MAGIC          0x464C457F
#define EXE_HEADER_ENTRY_IDX      6
#define EXE_HEADER_MAGICNUM_IDX   0

#define SYSCALL_HALT_NUM          1
#define SYSCALL_EXECUTE_NUM       2
#define SYSCALL_READ_NUM          3
#define SYSCALL_WRITE_NUM         4
#define SYSCALL_OPEN_NUM          5
#define SYSCALL_CLOSE_NUM         6
#define SYSCALL_GETARGS_NUM       7
#define SYSCALL_VIDMAP_NUM        8
#define SYSCALL_SETHANDLER_NUM    9
#define SYSCALL_SIGRETURN_NUM     10

// halt
int32_t sys_halt(uint8_t status);

// execute
int32_t sys_execute(const uint8_t* command);

// read
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

// write
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);

// open
int32_t sys_open(const uint8_t* filename);

// close
int32_t sys_close(int32_t fd);

// get arguments
int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

// map video
int32_t sys_vidmap(uint8_t** screen_start);

// set handler
int32_t sys_set_handler(int32_t signum, void* handler_address);

// sigreturn
int32_t sys_sigreturn(void);

// execute call
int32_t do_syscall(int32_t number, int32_t arg1, int32_t arg2, int32_t arg3);

// execute
int32_t do_execute(uint8_t *command);

#endif // SYSCALLS_H
