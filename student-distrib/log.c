/**
 * log.c
 *
 * vim:ts=4 expandtab
 */
#include "log.h"

const static char* log_level_string[4] = {"DEBUG", "INFO", "WARN", "ERROR"};

/*
 * log(LogLevel level, const char* msg, const char* func_name)
 * Description: logs errors
 * Inputs: level - log level, msg - message for log, func_name - name of function
 * Outputs: none
 */
void log(LogLevel level, const char* msg, const char* func_name) {
    if(level < LOG_LEVEL) {
        return;
    }

    printf("[%s] %s | %s\n", func_name, log_level_string[level], msg);
}

