#ifndef __UITLS_H__
#define __UITLS_H__
 
#include <stdint.h>
#include <stdio.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <sys/time.h>

const int MAX_LOG_BUFF_LEN = 2 * 1024;
const int MAX_INT_BUFF_LEN = 32;

#define LOG_PRINTF(fmt, ...) do{ \
    char buf[128] = {0}; \
    snprintf(buf, 128, "[%s - %s]", __FILE__, __FUNCTION__); \
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

#endif

