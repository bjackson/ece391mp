/**
 * syscalls.h
 *
 * vim:ts=4 expandtab
 */
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../types.h"
#include "../lib.h"

//
extern int32_t sys_halt(uint8_t status);

//
extern int32_t sys_execute(const uint8_t* command);

//
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

//
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);

//
extern int32_t sys_open(const uint8_t* filename);

//
extern int32_t sys_close(int32_t fd);

//
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

//
extern int32_t sys_vidmap(uint8_t** screen_start);

#endif // SYSCALLS_H
