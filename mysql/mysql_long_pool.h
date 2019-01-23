﻿#ifndef __MYSQL_LONG_POOL_H__
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
    MysqlLongPool(char* strPath, char* strUser, char* strPwd, int PoolSize, unsigned int read_timeout = 0);
    
    virtual ~MysqlLongPool(void);

    bool init();

    bool get_connection(ConnectionGuard<MySQLConnection> * connection, int timeout_ms = 0);

    void ping_for_pool();
    
    void ping_for_pool_impl();

private:

    int _pool_size;
    unsigned int _read_timeout;
    
    char* m_Path;
    char* m_User;
    char* m_Pwd;

    volatile bool _init_done;

    boost::thread* _ping_thread;
};

}
}
#endif
