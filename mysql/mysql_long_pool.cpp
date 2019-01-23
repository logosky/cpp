#include "db_pch.h"
#include "mysql_long_pool.h"
#include <mysql/mysql.h>

namespace demo
{
namespace mysql
{

MysqlLongPool::MysqlLongPool(char* strPath, char* strUser, char* strPwd, int PoolSize, unsigned int read_timeout)
    : ConnectionPool<MySQLConnection>(), 
    _pool_size(PoolSize),
    _read_timeout(read_timeout),
    _init_done(false)
{
    m_Path = strdup(strPath);
    m_User = strdup(strUser);
    m_Pwd = strdup(strPwd);

    _ping_thread = new boost::thread(boost::bind(&MysqlLongPool::ping_for_pool, this));
}

MysqlLongPool::~MysqlLongPool(void)
{
    _ping_thread->interrupt();
    _ping_thread->join();
    delete _ping_thread;
    
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

        if(_read_timeout > 0)
        {
            connection->SetReadTimeout(_read_timeout);
        }
        
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

void MysqlLongPool::ping_for_pool()
{
    pthread_setname_np(pthread_self(), "MYSQL_PING_THREAD");
    
    while(1)
    {
        ping_for_pool_impl();
    }
}

void MysqlLongPool::ping_for_pool_impl()
{
    LOG_PRINTF("doing ping");
    
    sleep(60);
    
    if(!_init_done)
    {
        return;
    }
    
    pthread_mutex_lock(&_mutex);
    if (_connections.empty())
    {
        pthread_mutex_unlock(&_mutex);
        
        LOG_PRINTF("pool is empty now, not need ping.");

        return;
    }

    std::vector<MySQLConnection*> temp_pool;

    while(!_connections.empty())
    {
        MySQLConnection* conn = _connections.front();
        _connections.pop();

        int ret = conn->Ping();
        if(ret != 0)
        {
            LOG_PRINTF("ping err:%d.", ret);
        }
        
        temp_pool.push_back(conn);
    }

    for(int i = 0; i < temp_pool.size();i++)
    {
        MySQLConnection* temp_conn = temp_pool[i];
        give_back(temp_conn);
    }
    pthread_mutex_unlock(&_mutex);
}

}
}
