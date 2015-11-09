/**
 * syscalls.c
 *
 * vim:ts=4 expandtab
 */
#include "syscalls.h"

/**
 *
 */
extern int32_t sys_halt(uint8_t status) {
    printf("Halt!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_execute(const uint8_t* command) {
    printf("Execute!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    printf("Read!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    printf("Write!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_open(const uint8_t* filename) {
    printf("Open!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_close(int32_t fd) {
    printf("Close!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    printf("Get Args!\n");
    return -1;
}

/**
 *
 */
extern int32_t sys_vidmap(uint8_t** screen_start) {
    printf("Vidmap!\n");
    return -1;
}

