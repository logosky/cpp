#include "hiredis_connection_pool.h"

#include "connection_pool.h"

namespace demo
{


HiredisConnectionPool::HiredisConnectionPool(int conn_size,
        const std::string & host,
        int port,
        const std::string & passwd,
        int db_index) :
    ConnectionPool<demo::hiredis::Client>(),
    _conn_size(conn_size),
    _host(host),
    _port(port),
    _passwd(passwd),
    _db_index(db_index)
{
}

HiredisConnectionPool::~HiredisConnectionPool()
{
    for (int i = 0; i < _conn_size; i++)
    {
        ConnectionGuard< demo::hiredis::Client> connection;
        if (ConnectionPool<demo::hiredis::Client>::get_connection(&connection))
        {
            demo::hiredis::Client * redis_conn = connection.release();
            delete redis_conn;
        }
    }
}

bool HiredisConnectionPool::init()
{
    std::vector< demo::hiredis::Client *> redis_connections;
    for (int i = 0; i < _conn_size; i++)
    {
        demo::hiredis::Client * connection = new demo::hiredis::Client(_host, _port, _passwd, _db_index);
        if (!connection->CheckConnect())
        {
            LOG_PRINTF("HiredisConnectionPool CheckConnect fail, %s:%d pwd:%s idx:%d\n", 
                _host.c_str(), 
                _port,
                _passwd.c_str(),
                _db_index);
            delete connection;
            return false;
        }

        LOG_PRINTF("HiredisConnectionPool connect succ, %s:%d pwd:%s idx:%d\n", 
            _host.c_str(), 
            _port,
            _passwd.c_str(),
            _db_index);

        redis_connections.push_back(connection);
    }

    add_connections(redis_connections);
    return true;
}

} 
