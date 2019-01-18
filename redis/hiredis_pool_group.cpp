#include "hiredis_pool_group.h"

#include "hiredis_connection_pool.h"
#include "redis_config.h"

namespace demo
{

HiredisPoolGroup::HiredisPoolGroup(int conn_size, const std::vector<RedisConfig> & vec_redis_config) : _vec_redis_config(vec_redis_config)
{
    _init_done = false;
    _conn_size = conn_size;
}

HiredisPoolGroup::~HiredisPoolGroup()
{
    for (std::vector<HiredisConnectionPool *>::iterator it = _redis_pools.begin(); it != _redis_pools.end(); ++it)
    {
        delete (*it);
    }
    _redis_pools.clear();
}

bool HiredisPoolGroup::init()
{
    LOG_PRINTF("init:%d, conn_size:%d, config size:%d\n", 
        _init_done,
        _conn_size,
        _vec_redis_config.size());
    
    if (_init_done)
    {
        return false;
    }

    if (_conn_size <= 0 || _vec_redis_config.size() <= 0)
    {
        return false;
    }

    for (std::vector<RedisConfig>::iterator it = _vec_redis_config.begin();
            it != _vec_redis_config.end(); ++it)
    {
        HiredisConnectionPool * redis_pool = new HiredisConnectionPool(_conn_size, it->_redis_host, it->_redis_port, it->_redis_passwd);
        if (!redis_pool->init())
        {
            LOG_PRINTF("init redis pool failed, %s:%d\n", it->_redis_host.c_str(), it->_redis_port);
            delete redis_pool;
            return false;
        }

        LOG_PRINTF("create HiredisConnectionPool success, conn size:%d\n", _conn_size);

        _redis_pools.push_back(redis_pool);
    }

    _init_done = true;

    return true;
}

bool HiredisPoolGroup::get_connection(ConnectionGuard<demo::hiredis::Client> * connection, int key, int timeout_ms)
{
    if (!_init_done)
    {
        return false;
    }

    int index = key % _redis_pools.size();
    return _redis_pools[index]->get_connection(connection, timeout_ms);
}

}
