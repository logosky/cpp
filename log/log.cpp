#include "log.h"

extern void log_fun(const char *file , int line , const char *func, const char* fmt, ...)
{
    char logbuffer[512] = {0}; 
    int n = snprintf(logbuffer, sizeof(logbuffer) - 1, "[%s] - [%s] - [%u] ", file, func, line); 
    va_list arglist; 
    va_start (arglist, fmt); 
    vsnprintf(logbuffer + n, sizeof(logbuffer) - n - 1, fmt, arglist); 
    va_end(arglist); 

    printf("%s\n", logbuffer); 
}

