/*
 * 遍历目录，删除指定时间之前创建的文件
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <tchar.h>
#endif

const char* LogFileName = "test.log";
const uint32_t LogExpireTime = 86400;

#ifdef _WIN32
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tv->tv_sec = static_cast<long>(clock);
    tv->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif

uint32_t get_seconds()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec;
}


bool clear_old_log(std::string& log_dir)
{
    uint32_t now_time = get_seconds();
    
#ifdef _WIN32
    _finddata_t file;
    intptr_t fd;
    std::string file_dir = log_dir + "*";
    fd = _findfirst(file_dir.c_str(), &file);
    if (fd == -1)
    {
        return false;
    }

    while (_findnext(fd, &file) == 0)
    {
        if(strncmp(file.name, LogFileName, strlen(LogFileName)) == 0)
        {
            std::string filename = log_dir + file.name;
            
            if(now_time > file.time_create + LogExpireTime)
            {
                DeleteFile(filename.c_str());
            }
        }
    }

    _findclose(fd);
#else
    DIR *dir;
    struct dirent *dirent_ptr;
    dir = opendir(log_dir.c_str());
    if(!dir)
    {
        return false;
    }

    do
    {
        dirent_ptr = readdir(dir);
        if(!dirent_ptr)
        {
            break;
        }
        
        if(dirent_ptr->d_type == DT_REG
            && (strncmp(dirent_ptr->d_name, LogFileName, strlen(LogFileName)) == 0))
        {
            std::string filename = _log_dir + dirent_ptr->d_name;
            
            struct stat file_stat;
            if(stat(filename.c_str(), &file_stat) == 0)
            {
                if(now_time > file_stat.st_mtime + LogExpireTime)
                {
                    remove(filename.c_str());
                }
            }
        }
    }while(1);
    
    closedir(dir);
#endif
    return true;
}
