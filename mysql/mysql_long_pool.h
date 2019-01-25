#ifndef __MYSQL_LONG_POOL_H__
#define __MYSQL_LONG_POOL_H__

#include "mysql_comm.h"
#include "mysql_interface.h"
#include "mysql_connect.h"
#include "connection_pool.h"

#include <list>
#include <boost/thread.hpp>

#pragma once


namespace demo
{
namespace mysql
{


class MysqlLongPool : public ConnectionPool<MySQLConnection>
{
public:
    // read_timeout:数据库操作最长等待时长，单位s
    MysqlLongPool(std::string strPath, std::string strUser, std::string strPwd, int PoolSize, unsigned int read_timeout = 0);
    
    virtual ~MysqlLongPool(void);

    bool init();

    // timeout_ms:从连接池获取连接的最长等候时长，单位ms
    bool get_connection(ConnectionGuard<MySQLConnection> * connection, int timeout_ms = 0);

    void ping_for_pool();
    
    void ping_for_pool_impl();

private:

    int _pool_size;
    unsigned int _read_timeout;  // 数据库操作最长等待时长，单位s
    
    std::string m_Path;
    std::string m_User;
    std::string m_Pwd;

    volatile bool _init_done;

    boost::thread* _ping_thread;
};

}
}
#endif
