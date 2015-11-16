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

#define EXE_HEADER_LEN 40
#define EXE_HEADER_MAGIC 0x464C457F

//
int32_t sys_halt(uint8_t status);

//
int32_t sys_execute(const uint8_t* command);

//
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

//
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);

//
int32_t sys_open(const uint8_t* filename);

//
int32_t sys_close(int32_t fd);

//
int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

//
int32_t sys_vidmap(uint8_t** screen_start);

//
int32_t debug_do_call(int32_t number, int32_t arg1, int32_t arg2, int32_t arg3);

#endif // SYSCALLS_H
