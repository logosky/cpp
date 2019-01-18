
#include "redis_config.h"

#include <string.h>
#include <fstream>
#include <json/json.h>

namespace demo
{

RedisConfigLoad::RedisConfigLoad(const char * config_name) : _config_name(config_name)
{

}

RedisConfigLoad::~RedisConfigLoad()
{

}

bool RedisConfigLoad::load_config()
{
    if (_config_name.empty())
    {
        return false;
    }

    std::ifstream ifs;
    ifs.open(_config_name.c_str(), std::ios::in);

    if (!ifs.is_open())
    {
        return false;
    }

    ifs.seekg(0, std::ios_base::end);
    size_t size = ifs.tellg();
    char * str_config = new char[size+1];
    ifs.seekg(0, std::ios_base::beg);
    ifs.read(str_config, size);
    
    str_config[size] = '\0';

    bool bret = parse_config(str_config);

    delete []str_config;
    ifs.close();

    return bret;
}

#define SAFE_GET_JSON_STRING( jsn_node, field, config_var ) do { \
    if (!jsn_node[field].isString()) \
    { return false; } \
    config_var = jsn_node[field].asString(); \
} while(0)

#define SAFE_GET_JSON_INT( jsn_node, field, config_var ) do { \
    if (!jsn_node[field].isInt()) \
    { return false; } \
    config_var = jsn_node[field].asInt(); \
} while(0)

// 配置文件形式
/*
 *
 * {
      "ThreadCount": 4, // 每个线程池中有多少个线程
      "TotalRedisConfigCount": 2, // 总共有多少个redis配置项目，即RedisConfig个数
      "RedisConfig0": {
      "Ip": "",
       "Port": ,
       "Passwd": "",
       "Prefix": ""
       },
       "RedisConfig1": {
       "Ip": "",
       "Port": ,
       "Passwd": "",
       "Prefix": ""
       }
   }
   // 如果有更多的RedisConfig添加索引即可,例如要添加3个则配置为RedisConfig2
   // 有更多的配置则依次类推
 */
bool RedisConfigLoad::parse_config(const char * str_config)
{
    Json::Value config_root;
    Json::Reader r;

    if (!r.parse(str_config, config_root))
    {
        return false;
    }

    const char * const THREAD_COUNT = "ThreadCount";
    const char * const TOTAL_REDIS_CONFIG_COUNT = "TotalRedisConfigCount";
    const char * const REDIS_CONFIG_BASE = "RedisConfig";

    SAFE_GET_JSON_INT( config_root, THREAD_COUNT, _thread_count );

    int total_redis_config_count = 0;
    SAFE_GET_JSON_INT( config_root, TOTAL_REDIS_CONFIG_COUNT, total_redis_config_count );

    _vec_redis_config.reserve(total_redis_config_count);

    char json_key[128] = { 0 };
    for (int i = 0; i < total_redis_config_count; ++i)
    {
        memset(json_key, 0, sizeof(json_key));
        snprintf(json_key, sizeof(json_key),
                "%s%d", 
                REDIS_CONFIG_BASE, i);

        Json::Value & jsn_node = config_root[json_key];

        RedisConfig redis_config;

        if (!parse_redis_config_impl(jsn_node, redis_config))
        {
            return false;
        }

        _vec_redis_config.push_back(redis_config);
    }

    return true;
}

bool RedisConfigLoad::parse_redis_config_impl(Json::Value & jsn_redis_config,
            RedisConfig & redis_config)
{
    SAFE_GET_JSON_STRING( jsn_redis_config, "Ip", redis_config._redis_host );
    SAFE_GET_JSON_INT( jsn_redis_config, "Port", redis_config._redis_port );
    SAFE_GET_JSON_STRING( jsn_redis_config, "Passwd", redis_config._redis_passwd );
    SAFE_GET_JSON_STRING( jsn_redis_config, "Prefix", redis_config._redis_prefix );

#undef SAFE_GET_JSON_STRING
#undef SAFE_GET_JSON_INT
    return true;
}

}
