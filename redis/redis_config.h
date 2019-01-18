
#ifndef __REDIS_CONFIG_H__
#define __REDIS_CONFIG_H__

#include <stdint.h>
#include <string>
#include <vector>

namespace Json
{
    class Value;
}

namespace demo
{

struct RedisConfig
{
    RedisConfig() :
        _redis_port(0)
    { }

    std::string _redis_host;
    uint16_t _redis_port;
    std::string _redis_passwd;
    std::string _redis_prefix;
};

class RedisConfigLoad
{
public:
    RedisConfigLoad(const char * config_name);
    ~RedisConfigLoad();

    bool load_config();

    const std::vector<RedisConfig> & get_redis_config() const 
    {
        return _vec_redis_config;
    }

    int get_thread_count() const { return _thread_count; }
private:
    bool parse_config(const char * str_config);

    bool parse_redis_config_impl(Json::Value & jsn_redis_config,
            RedisConfig & redis_config);
private:
    std::string _config_name;

    int _thread_count;
    std::vector<RedisConfig> _vec_redis_config;
};

}
#endif

