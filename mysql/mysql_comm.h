#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#ifndef __MYSQL_COMM_H__
#define __MYSQL_COMM_H__



#ifdef WIN32
#include <tchar.h>
#endif

// Standard constants
#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0




#ifndef IN
/// 用于指示函数的函数为输入参数,对编译无影响
#define IN
#endif

#ifndef OUT
/// 用于指示函数的参数为输出参数,对编译无影响
#define OUT
#endif


/// 用于指示函数的函数为输入输出参数,对编译无影响
#define IN_OUT

/// 用于指示函数的函数为可选输入参数,对编译无影响
#define IN_OPT

/// 用于指示函数的函数为可选输出参数,对编译无影响
#define OUT_OPT

/// 用于指示函数的函数为可选输入输出参数,对编译无影响
#define IN_OUT_OPT


#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#ifdef UNICODE
#define __WFILE__ WIDEN(__FILE__)
#else
#define __WFILE__  L ## __FILE__
#endif




#ifdef __cplusplus
/** \namespace DBCommon
*   公共定义的命名空间
*
*/
namespace demo
{
namespace mysql
{
#endif



typedef unsigned char       UInt8;
typedef short               Int16;
typedef unsigned short      UInt16;
typedef int                 Int32;
typedef unsigned int        UInt32;
typedef long long           Int64;
typedef unsigned long long  UInt64;
typedef unsigned int        UInt;


typedef char                Int8;
typedef Int32               Int;

typedef long                Long;
typedef unsigned long       ULong;
typedef UInt8               Byte;
typedef Int32               Bool;

typedef float               Float;
typedef double              Double;
typedef double              DOUBLE;

typedef char *              String;



const int MAX_Path_LEN          = 260;


#if !defined(_MSC_VER)
#define _T(x)      x
#define _tstoi      atoi
#endif


/**
*@brief    数据库模块接口调用通用返回值
*
*数据库模块接口调用通用返回值
*/
enum RCode
{

    rcFaild                    = 0,          ///< 不明原因的失败

    rcSuccess                  = 1,          ///< 成功

    rcWrongParameter           = 0x10000000, ///< 传入参数不正确

    rcNoResult                 = 0x20000,   ///< 数据库未返回结果

    rcDBAccessError            = 0x20001,   ///< 访问数据库错误

    rcNoneInserted             = 0x20002,   ///< 未能插入任何数据

    rcNoneUpdated              = 0x20003,   ///< 未能更新任何数据

    rcNoneDeleted              = 0x20004,   ///< 未能删除任何数据

    rcCapacityOverflow         = 0x20005,   ///< 超出设计容量

    rcNotAllClosed             = 0x30001,   ///< 未关闭所有的数据库

    rcCOMInitFaild             = 0x30002,   ///< 初始化COM环境失败

    rcDBFileNotExist           = 0x30003,   ///< 指定的数据库文件不存在

    rcCreateADOConnectionFaild = 0x30004,    ///< 创建ADODB.Connection失败

    rcOpenConnectionFaild      = 0x30005,    ///< 打开数据库连接失败

    rcDBAuthError              = 0x30006,     ///< 数据库验证错误（用户名或密码错误）

    rcOpenFileFailed            = 0x40001,  ///< 无法打开文件

    rcWriteFileFailed           = 0x40002,   ///< 写文件出错

    rcWriteFileLess,
    rcReadFileFailed,
    rcReadFileLess,

    rcFreeSpaceLack,                        ///< 空闲空间不足

    rcConnectDeviceFaild,                   ///< 连接设备失败
    rcDisconnectDeviceFaild,                ///< 断开设备失败
    rcGetRemoteFileFaid,                    ///< 获取远程文件失败
    rcPutRemoteFileFaid,                    ///< 写入远程文件失败
    rcDeleteFileFaid,                       ///< 删除远程文件失败
    rcCallRemoteInterfaceFaild,             ///< 无法调用远程接口
    rcRemoteInterfaceError,                 ///< 远程接口返回错误
    rcOpenMainDBFaid,                       ///< 打开主数据库失败
    rcOpenLogDBFaid,                        ///< 打开日志数据库失败
    rcOpenDBFaid,                           ///< 打开数据库失败
    rcExportDataError,                      ///< 导出数据失败





    rcDetectedDuplicate,                    ///< 检测到冲突
    rcNotDetectedDuplicate,                 ///< 未检测到冲突


    rcEnd

};






/**
*@brief 日期时间结构体
*
*日期时间结构体
*/
typedef struct _DBDateTime
{
    Int Year; ///<  年
    Int Month; ///<  月
    Int Day; ///<  日
    Int Hour; ///<  时
    Int Minute; ///<  分
    Int Second; ///<  秒
    Int Milliseconds; ///<  微秒
    Int TimeZone; ///<  时区 ， >=13, <=-13 表示不确定.
} DBDateTime;



















#ifndef memclr
#define memclr(x) memset(&x, 0, sizeof(x))
#endif

#ifdef __cplusplus
}
} //namespace
#endif

#endif // !defined(_DB_COMMON_DEF_H)
