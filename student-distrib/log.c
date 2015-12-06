/**
 * log.c
 *
 * vim:ts=4 expandtab
 */
#include "log.h"

const static char* log_level_string[4] = {"DEBUG", "INFO", "WARN", "ERROR"};

void log(LogLevel level, const char* msg, const char* func_name) {
    if(level < LOG_LEVEL) {
        return;
    }

    printf("[%s] %s | %s\n", func_name, log_level_string[level], msg);
}

