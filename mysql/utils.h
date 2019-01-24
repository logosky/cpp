#ifndef __UITLS_H__
#define __UITLS_H__
 
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <sstream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <signal.h>
#include <sys/resource.h>

const int MAX_LOG_BUFF_LEN = 2 * 1024;
const int MAX_INT_BUFF_LEN = 32;

#define LOG_PRINTF(fmt, ...) do{ \
    char buf[128] = {0}; \
    snprintf(buf, 128, "[%s - %s - %d]", __FILE__, __FUNCTION__, __LINE__); \
    log_printf(buf, fmt, ##__VA_ARGS__); \
}while(0)

class Read_Write_Lock
{
public:
    Read_Write_Lock()
    {

    };

    ~Read_Write_Lock()
    {

    };

    void lock_read()
    {
        rw_lock.lock_shared();
    }

    void unlock_read()
    {
        rw_lock.unlock_shared();
    }

    void lock_write()
    {
        rw_lock.lock();
    }

    void unlock_write()
    {
        rw_lock.unlock();
    }

private:
    boost::shared_mutex rw_lock;

};


class Auto_Read_Lock
{
public:

    Auto_Read_Lock(Read_Write_Lock & rw):
        rwlock(rw)
    {
        rwlock.lock_read();
    }

    ~Auto_Read_Lock()
    {
        rwlock.unlock_read();
    }

protected:
    Read_Write_Lock & rwlock;
};


class Auto_Write_Lock
{
public:
    Auto_Write_Lock(Read_Write_Lock & rw):
        rwlock(rw)
    {
        rwlock.lock_write();
    }

    ~Auto_Write_Lock()
    {
        rwlock.unlock_write();
    }

protected:
    Read_Write_Lock & rwlock;
};

void log_printf(const char* base_info, const char* fmt, ...);

void get_current_readable_time(char * readable_time, int len);

inline uint64_t get_cur_time_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    uint64_t cur_time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return cur_time_ms;
}

inline uint64_t get_cur_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    uint64_t cur_time_us = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;

    return cur_time_us;
}

inline uint64_t get_tick_count()
{
    //volatile static uint64_t lastTick = 0;
    uint64_t currentTime;

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#ifdef _SERVER
    uint64_t currentTime2;

    currentTime = GetTickCount64();
    currentTime2 = timeGetTime();

    currentTime  &= 0xffffffffffffff00;
    currentTime2 &= 0x00000000000000ff;
    currentTime  |= currentTime2;
#else
    currentTime = timeGetTime();
#endif

#endif


#if defined(linux) || defined(__linux) || defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    currentTime = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif

    return currentTime;
}


inline uint64_t get_us_tick_count()
{
    uint64_t currentTime;

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

    static LARGE_INTEGER TicksPerSecond = {0};
    LARGE_INTEGER Tick;

    if (!TicksPerSecond.QuadPart)
    {
        QueryPerformanceFrequency(&TicksPerSecond);
    }

    QueryPerformanceCounter(&Tick);

    int64_t Seconds = Tick.QuadPart / TicksPerSecond.QuadPart;
    int64_t LeftPart = Tick.QuadPart - (TicksPerSecond.QuadPart * Seconds);
    int64_t uSeconds = LeftPart * 1000 * 1000 / TicksPerSecond.QuadPart;
    currentTime = Seconds * 1000 * 1000 + uSeconds;
#endif


#if defined(linux) || defined(__linux) || defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    currentTime = (ts.tv_sec * 1000 * 1000 + ts.tv_nsec / 1000);
#endif

    return currentTime;
}

void daemon_init_stdout_open();

#endif

