#include "utils.h"
#include <stdarg.h>

using namespace std;

static pthread_mutex_t LOCK_localtime_r;

void log_printf(const char* base_info, const char* fmt, ...)
{
    int len = 0;
    char log_buff[MAX_LOG_BUFF_LEN] = {0};

    char time_ms[MAX_INT_BUFF_LEN] = {0};
    get_current_readable_time(time_ms, MAX_INT_BUFF_LEN);

    snprintf(log_buff, MAX_LOG_BUFF_LEN, "%s %s ", time_ms, base_info);
    len = strlen(log_buff);
    
    va_list va_arg;
    va_start(va_arg, fmt);
    int serialize_fmt = vsnprintf(log_buff + len ,MAX_LOG_BUFF_LEN - len, fmt, va_arg);
    va_end(va_arg);

    printf("%s\n", log_buff);
}

struct tm *localtime_r(const time_t *clock, struct tm *res)
{
  struct tm *tmp;
  pthread_mutex_lock(&LOCK_localtime_r);
  tmp=localtime(clock);
  *res= *tmp;
  pthread_mutex_unlock(&LOCK_localtime_r);
  return res;
}

void get_current_readable_time(char * readable_time, int len)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    int ms = tv.tv_usec / 1000;
    time_t cur_time_sec = tv.tv_sec;
    struct tm tm_res;
    localtime_r(&cur_time_sec, &tm_res);

    snprintf(readable_time, len, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            tm_res.tm_year + 1900,
            tm_res.tm_mon + 1,
            tm_res.tm_mday,
            tm_res.tm_hour,
            tm_res.tm_min,
            tm_res.tm_sec,
            ms);
}

