#include<stdio.h>

extern void log_fun(const char *file , int line , const char *func, const char* fmt, ...);

#define FLOW_LOG(fmt, args...) do{ \
 log_fun(__FILE__ , __LINE__ , __FUNCTION__, fmt, ##args);\
}while(0);

#define DEBUG_LOG(fmt, args...)  do{ \
 log_fun(__FILE__ , __LINE__ , __FUNCTION__, fmt, ##args);\
}while(0);

#define ERROR_LOG(fmt, args...)  do{ \
 log_fun(__FILE__ , __LINE__ , __FUNCTION__, fmt, ##args);\
}while(0);

#define WARNING_LOG(fmt, args...)  do{ \
 log_fun(__FILE__ , __LINE__ , __FUNCTION__, fmt, ##args);\
}while(0);

#define NOTICE_LOG(fmt, args...)  do{ \
 log_fun(__FILE__ , __LINE__ , __FUNCTION__, fmt, ##args);\
}while(0);

