#include "db_pch.h"
#include "mysql_long_pool.h"
#include <mysql/mysql.h>

namespace demo
{
namespace mysql
{

MysqlLongPool::MysqlLongPool(char* strPath, char* strUser, char* strPwd, int PoolSize)
    : ConnectionPool<MySQLConnection>(), 
    _pool_size(PoolSize),
    _init_done(false)
{
    m_Path = strdup(strPath);
    m_User = strdup(strUser);
    m_Pwd = strdup(strPwd);
}

MysqlLongPool::~MysqlLongPool(void)
{
    for (int i = 0; i < _pool_size; i++)
    {
        ConnectionGuard< MySQLConnection > connection;
        if (ConnectionPool<MySQLConnection>::get_connection(&connection))
        {
            MySQLConnection * conn = connection.release();
            if(conn)
            {
                conn->Close();
                delete conn;
            }
            mysql_thread_end();
        }
    }
    
    if (m_Path)
    {
        free(m_Path);
    }
    
    if (m_User)
    {
        free(m_User);
    }

    if (m_Pwd)
    {
        free(m_Pwd);
    }
}

bool MysqlLongPool::init()
{
    std::vector< MySQLConnection *> mysql_connections;
    for (int i = 0; i < _pool_size; i++)
    {
        MySQLConnection * connection = new MySQLConnection();
        
        connection->SetAccessUrl(m_Path);
        connection->SetPass(m_User, m_Pwd);
        
        if (!connection->Open())
        {
            LOG_PRINTF("MysqlLongPool connect fail, %s user:%s pwd:%s", 
                m_Path, 
                m_User,
                m_Pwd);
            
            delete connection;
            return false;
        }

        LOG_PRINTF("MysqlLongPool connect succ, %s user:%s pwd:%s", 
            m_Path, 
            m_User,
            m_Pwd);

        mysql_connections.push_back(connection);
    }

    add_connections(mysql_connections);

    _init_done = true;
    
    return true;
}

bool MysqlLongPool::get_connection(ConnectionGuard<MySQLConnection> * connection, int timeout_ms)
{
    if(!_init_done)
    {
        LOG_PRINTF("MysqlLongPool not init down yet, %s user:%s pwd:%s", 
            m_Path, 
            m_User,
            m_Pwd);
        
        return false;
    }

    return ConnectionPool<MySQLConnection>::get_connection(connection, timeout_ms);
}

}
}
