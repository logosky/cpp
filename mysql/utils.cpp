#include "utils.h"
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

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


void sig_handler(int signum)
{
    switch(signum)
    {
    case SIGHUP:
        break;
    case SIGINT:
    case SIGPIPE:
    case SIGALRM:
    case SIGTERM:
    case SIGPOLL:
    case SIGPROF:
        // 捕获异常退出，调exit(0)会清理资源，目前服务线程模型无法优雅退出，必然会coredump
        _exit(0);
        break;
    case SIGQUIT:
    case SIGILL:
    case SIGABRT:
    case SIGFPE:
    case SIGSEGV:
    case SIGBUS:
    case SIGSYS:
    case SIGTRAP:
    case SIGXCPU:
    case SIGXFSZ:
        signal(signum, SIG_DFL);
        kill(getpid(), signum);
        break;
    default:
        break;
    }
}


void daemon_init_stdout_open()
{
    int pid;
    struct rlimit   rl;                       //获取进程资源西限制
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)    //获取进程最多文件数
    {
        printf(":can't get file limit");
    }
    pid = fork();
    if (pid)
    {
        exit(0);                          //父进程，退出
    }
    else if (pid < 0)                         //开辟进程失败，退出并关闭所有进程
    {
        exit(1);
    }

    /* 子进程继续执行 */
    setsid();                               //创建新的会话组，子进程成为组长，并与控制终端分离
    /* 防止子进程（组长）获取控制终端 */
    pid = fork();
    if (pid)
    {
        exit(0);                       //父进程，退出
    }
    else if (pid < 0)
    {
        exit(1);                        //开辟进程失败，退出并关闭所有进程
    }

// 不能关闭输入输出
#if 0
    /* 第二子进程继续执行 , 第二子进程不再是会会话组组长*/
    /* 关闭打开的文件描述符*/
    if (rl.rlim_max == RLIM_INFINITY)     //RLIM_INFINITY是一个无穷量的限制
    {
        rl.rlim_max = 1024;

    }

    for (int i = 0; i < (int)rl.rlim_max; i++)
    {
        if (i == STDOUT_FILENO || i == STDIN_FILENO)
        {
            continue;
        }
        close(i);

    }
#endif // #if 0

    umask(0);       // 重设文件创建掩码

    signal(SIGHUP, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGPIPE, sig_handler);
    signal(SIGALRM, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGPOLL, sig_handler);
    signal(SIGPROF, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGILL, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGBUS, sig_handler);
    signal(SIGSYS, sig_handler);
    signal(SIGTRAP, sig_handler);
    signal(SIGXCPU, sig_handler);
    signal(SIGXFSZ, sig_handler);
    
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
}


