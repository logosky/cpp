#ifndef __HIREDIS_POOL_GROUP_H__
#define __HIREDIS_POOL_GROUP_H__

#include <vector>

#include "hiredisclient.h"

#include "connection_pool.h"
#include "redis_config.h"

namespace demo
{

class HiredisConnectionPool;
class HiredisPoolGroup
{
public:
    HiredisPoolGroup(int conn_size, const std::vector<RedisConfig> & vec_redis_config);
    ~HiredisPoolGroup();

    bool init();

    bool get_connection(ConnectionGuard<demo::hiredis::Client> * connection, int key = 0, int timeout_ms = 0);

private:
    volatile bool _init_done;
    int _conn_size;
    std::vector<RedisConfig> _vec_redis_config;
    std::vector<HiredisConnectionPool *> _redis_pools;
};

} 

#endif 
