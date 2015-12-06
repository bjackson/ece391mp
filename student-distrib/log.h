/**
 * log.h
 *
 * vim:ts=4 expandtab
 */
#ifndef LOG_H
#define LOG_H

#include "lib.h"

#define LOG_LEVEL DEBUG

typedef enum {DEBUG,INFO,WARN,ERROR} LogLevel;

const static char* log_level_string[4] = {"DEBUG", "INFO", "WARN", "ERROR"};

void log(LogLevel level, const char*  msg, const char* func_name);

#endif