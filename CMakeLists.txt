cmake_minimum_required(VERSION 3.3)
project(mp3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    fish/blink.h
    fish/blink.S
    fish/ece391support.c
    fish/ece391support.h
    fish/ece391syscall.h
    fish/ece391sysnum.h
    fish/fish.c
    student-distrib/devices/i8259.c
    student-distrib/devices/i8259.h
    student-distrib/devices/rtc.c
    student-distrib/devices/rtc.h
    student-distrib/interrupts/interrupts.c
    student-distrib/interrupts/interrupts.h
    student-distrib/interrupts/interrupts_asm.S
    student-distrib/boot.S
    student-distrib/debug.sh
    student-distrib/kernel.c
    student-distrib/lib.c
    student-distrib/lib.h
    student-distrib/Makefile
    student-distrib/multiboot.h
    student-distrib/paging.c
    student-distrib/paging.h
    student-distrib/types.h
    student-distrib/x86_desc.h
    student-distrib/x86_desc.S
    syscalls/ece391cat.c
    syscalls/ece391counter.c
    syscalls/ece391emulate.c
    syscalls/ece391grep.c
    syscalls/ece391hello.c
    syscalls/ece391ls.c
    syscalls/ece391pingpong.c
    syscalls/ece391shell.c
    syscalls/ece391sigtest.c
    syscalls/ece391support.c
    syscalls/ece391support.h
    syscalls/ece391syscall.h
    syscalls/ece391syscall.S
    syscalls/ece391syserr.c
    syscalls/ece391sysnum.h
    syscalls/ece391testprint.c
    syscalls/Makefile)

add_executable(mp3 ${SOURCE_FILES})