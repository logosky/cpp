#ifndef __MYSQL_POOL_H__
#define __MYSQL_POOL_H__

#include "mysql_comm.h"
#include "mysql_interface.h"
#include "mysql_connect.h"

#include <list>
#include <boost/thread.hpp>

#pragma once


namespace demo
{
namespace mysql
{

enum DBType
{
    dbt_MySQL,
    dbt_MSSQL
};

class MysqlPool
{
public:
    MysqlPool(DBType dbt, char* strPath, char* strUser, char* strPwd, int PoolSize);
    virtual ~MysqlPool(void);


    /** @brief 从池中取出一个数据库连接，需调用CloseConn()释放
    *
    *     demo::mysql::MysqlPool::GetConn
    *     Access:    public
    *
    *     @return :   IConnection *
    */
    IConnection * GetConn();

    /** @brief 从池中取出一个数据库连接，需调用CloseConn()释放
    *
    *     demo::mysql::MysqlPool::GetConn
    *     Access:    public
    *
    *     @param:
    *       [in] read_timeout 请求MySQL的超时时间，单位为秒
    *       在MySQL客户端库代码层的超时时间是 read_timeout * 3
    *       即MySQL客户端代码会重试三次，
    *       具体参考文档：
    *       http://dev.mysql.com/doc/refman/5.7/en/mysql-options.html
    *       关于MYSQL_OPT_READ_TIMEOUT 说明
    *
    *     @return :   IConnection *
    */
    IConnection * GetConn(unsigned int read_timeout);

    /** @brief 从池中取出一个初始化成功的数据库连接
    *
    *     demo::mysql::MysqlPool::GetConnUseful
    *     Access:    public
    *      
    *     @param
    *       [in]:
    *           
    *       [out]:
    *
    *     @return :   成功时返回 IConnection *，否则返回 NULL
    */
    IConnection *GetConnUseful();

    /** @brief 从池中取出一个初始化成功的数据库连接,并设置读取的超时时间,单位为秒
    *
    *     demo::mysql::MysqlPool::GetConnUseful
    *     Access:    public
    *      
    *     @param
    *       [in]:read_timeout 读取的超时时间
    *           
    *     @return :   成功时返回 IConnection *，否则返回 NULL
    */
    IConnection *GetConnUseful(unsigned int read_timeout);

    /** @brief 将一个数据库连接归还到池中
    *
    *     demo::mysql::MysqlPool::CloseConn
    *     Access:    public
    *
    *     @param IConnection *
    *     @return :   void
    */
    void CloseConn(IConnection *);

    const int GetTotalCount();

private:
    boost::mutex m_Mutex;

    typedef std::list<IConnection *> ConnList;
    ConnList m_Conns;


    // 注释掉，避免编译警告未使用变量
    // int m_PoolSize;
    // DBType m_DBType;
    char* m_Path;
    char* m_User;
    char* m_Pwd;
    volatile int m_TotalCount;
};

}
}
#endif
