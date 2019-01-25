#include "db_pch.h"
#include "mysql_long_pool.h"
#include <mysql/mysql.h>

namespace demo
{
namespace mysql
{

MysqlLongPool::MysqlLongPool(std::string strPath, std::string strUser, std::string strPwd, int PoolSize, unsigned int read_timeout)
    : ConnectionPool<MySQLConnection>(), 
    _pool_size(PoolSize),
    _read_timeout(read_timeout),
    m_Path(strPath),
    m_User(strUser),
    m_Pwd(strPwd),
    _init_done(false)
{
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
}

bool MysqlLongPool::init()
{
    std::vector< MySQLConnection *> mysql_connections;
    for (int i = 0; i < _pool_size; i++)
    {
        MySQLConnection * connection = new MySQLConnection();
        
        connection->SetAccessUrl(m_Path.c_str());
        connection->SetPass(m_User.c_str(), m_Pwd.c_str());

        if(_read_timeout > 0)
        {
            connection->SetReadTimeout(_read_timeout);
        }
        
        if (!connection->Open())
        {
            LOG_PRINTF("MysqlLongPool connect fail, %s user:%s pwd:%s", 
                m_Path.c_str(), 
                m_User.c_str(),
                m_Pwd.c_str());
            
            delete connection;
            return false;
        }

        LOG_PRINTF("MysqlLongPool connect succ, %s user:%s pwd:%s", 
            m_Path.c_str(), 
            m_User.c_str(),
            m_Pwd.c_str());

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
            m_Path.c_str(), 
            m_User.c_str(),
            m_Pwd.c_str());
        
        return false;
    }

    return ConnectionPool<MySQLConnection>::get_connection(connection, timeout_ms);
}

void MysqlLongPool::ping_for_pool()
{
    pthread_setname_np(pthread_self(), "mysql_ping");
    
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

    uint64_t begin = get_us_tick_count();

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
        _connections.push(temp_conn);
        LOG_PRINTF("give back connection:%x, size:%d\n", temp_conn, _connections.size());
        pthread_cond_signal(&_cond);
    }

    uint64_t end = get_us_tick_count();
    LOG_PRINTF("ping use time:%llu\n", (end - begin));

    pthread_mutex_unlock(&_mutex);
}

}
}
