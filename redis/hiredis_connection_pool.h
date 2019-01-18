#ifndef __HIREDIS_CONNECTION_POOL_H__
#define __HIREDIS_CONNECTION_POOL_H__

#include <string>
#include <vector>
#include "hiredisclient.h"

#include "connection_pool.h"

namespace demo
{


class HiredisConnectionPool : public ConnectionPool<demo::hiredis::Client>
{
public:
    explicit HiredisConnectionPool(int conn_size,
            const std::string & host,
            int port,
            const std::string & passwd,
            int db_index = 0);

    virtual ~HiredisConnectionPool();

    bool init();

private:
    int _conn_size;
    std::string _host;
    int _port;
    std::string _passwd;
    int _db_index;
}; // class HiredisConnectionPool

} 

#endif 
