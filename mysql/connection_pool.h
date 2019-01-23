#ifndef __CONNECTION_POOL_H_
#define __CONNECTION_POOL_H_

#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <queue>

#include "utils.h"

namespace demo
{

template<typename Connection>
class ConnectionPool;

template<typename Connection>
class ConnectionGuard
{
public:
    friend class ConnectionPool<Connection>;

    ConnectionGuard(const std::string & tag = "") : _connection(NULL), _pool(NULL)
    {
        _create_time_ms = get_cur_time_ms();
        _get_connection_time_ms = 0;
    }

    ~ConnectionGuard()
    {
        LOG_PRINTF("ConnectionGuard::~ConnectionGuard\n");
        if (_pool && _connection)
        {
            _pool->give_back(_connection);

            int64_t now = get_cur_time_ms();
            int64_t wait_time_ms = _get_connection_time_ms - _create_time_ms;
            int64_t exe_time_ms = now - _get_connection_time_ms;
            int64_t cost_time_ms = now - _create_time_ms;
            if (cost_time_ms > 100)
            {
                LOG_PRINTF("give back connection:%x, tag:%s, wait_time:%dms, exe_time:%dms, cost_time:%dms\n",
                    _connection, _tag.c_str(), wait_time_ms, exe_time_ms, cost_time_ms);
            }
            else
            {
                LOG_PRINTF("give back connection:%x, tag:%s, wait_time:%dms, exe_time:%dms, cost_time:%dms\n",
                        _connection, _tag.c_str(), wait_time_ms, exe_time_ms, cost_time_ms);
            }
        }
    }

    operator bool () const 
    {
        return (_connection != NULL && _pool != NULL);
    }

    Connection * operator->() const
    {
        return _connection;
    }

    Connection * get() const
    {
        return _connection;
    }

    Connection * release()
    {
        Connection * prev = _connection;
        _connection = NULL;
        return prev;
    }

    void swap(Connection ** connection)
    {
        std::swap(_connection, *connection);
    }

private:
    void set_connection(Connection * connection, ConnectionPool<Connection> * pool) 
    {
        _connection = connection;
        _pool = pool;
        _get_connection_time_ms = get_cur_time_ms();
    }

    ConnectionGuard(const ConnectionGuard &);
    void operator=(const ConnectionGuard &);

private:
    Connection * _connection;
    ConnectionPool<Connection> * _pool;
    std::string _tag;
    int64_t _create_time_ms;
    int64_t _get_connection_time_ms;
}; // ConnectionGuard

template<typename Connection>
class ConnectionPool
{
public:
    ConnectionPool() : _init_done(false)
    {
        pthread_mutex_init(&_mutex, NULL);
        pthread_cond_init(&_cond, NULL);
    }

    virtual ~ConnectionPool()
    {

    }

    void add_connections(const std::vector<Connection*> & connections)
    {
        for (int i = 0; i < connections.size(); ++i)
        {
            give_back(connections[i]);
        }
        LOG_PRINTF("add_connections finished, add size:%d, total size:%d\n", connections.size(), _connections.size());
    }

    virtual bool get_connection(ConnectionGuard<Connection> * connection_guard, int timeout_ms = 0)
    {
        struct timespec tm;
        if (timeout_ms > 0)
        {
            struct timeval tmv;
            gettimeofday(&tmv, NULL);

            tm.tv_sec = tmv.tv_sec + timeout_ms / 1000;
            tm.tv_nsec = tmv.tv_usec * 1000 + (timeout_ms % 1000) * 1000 * 1000;

            if (tm.tv_nsec >= 1000 * 1000 * 1000)
            {
                ++tm.tv_sec;
                tm.tv_nsec -= 1000 * 1000 * 1000;
            }
        }

        LOG_PRINTF("get connection, timeout_ms:%d\n", timeout_ms);

        int err = 0;
        Connection * connection = NULL;
        pthread_mutex_lock(&_mutex);
        while (_connections.empty())
        {
            if (timeout_ms <= 0)
            {
                err = pthread_cond_wait(&_cond, &_mutex);
            }
            else
            {
                err = pthread_cond_timedwait(&_cond, &_mutex, &tm);
            }

            if (err == ETIMEDOUT)
            {
                break;
            }
        }

        LOG_PRINTF("wait finished, ret:%d\n", err);

        if (!_connections.empty())
        {
            connection = _connections.front();
            _connections.pop();
            LOG_PRINTF("get connection success:%x, size:%d\n", connection, _connections.size());
        }

        pthread_mutex_unlock(&_mutex);

        if (connection)
        {
            connection_guard->set_connection(connection, this);
            return true;
        }
        else
        {
            return false;
        }
    }

    void give_back(Connection * connection)
    {
        assert(connection);

        pthread_mutex_lock(&_mutex);
        _connections.push(connection);
        LOG_PRINTF("give back connection:%x, size:%d\n", connection, _connections.size());
        pthread_cond_signal(&_cond);
        pthread_mutex_unlock(&_mutex);
    }

protected:
    volatile bool _init_done;
    std::queue<Connection *> _connections;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;

}; // class ConnectionPool

} 

#endif 


