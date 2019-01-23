
#include "mysql_config.h"

#include <string.h>
#include <fstream>
#include <json/json.h>

namespace demo
{

namespace mysql
{

MysqlConfigLoad::MysqlConfigLoad(const char * config_name) :
    _config_name(config_name)
{
}

MysqlConfigLoad::~MysqlConfigLoad()
{
}

bool MysqlConfigLoad::load_config()
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
      "TotalMysqlConfigCount": 2, // 总共有多少个mysql配置项目，即MysqlConfig个数
      "MysqlConfig0": {
       "Host": "192.168.5.11",
       "Port": 6379,
       "DbName": "stt_activity",
       "User": "weinanchen"
       "Passwd": "weinanchen",
       "ConnectionPoolSize": 5
       },
       "MysqlConfig1": {
       "Host": "192.168.5.11",
       "Port": 6379,
       "DbName": "stt_activity",
       "User": "weinanchen"
       "Passwd": "weinanchen",
       "ConnectionPoolSize": 5
       }
   }
   // 如果有更多的MysqlConfig添加索引即可,例如要添加3个则配置为MysqlConfig2
   // 有更多的配置则依次类推
   // 注意：为了防止使用时出错，最好是一类数据库使用一个配置
 */


bool MysqlConfigLoad::parse_config(const char * str_config)
{
    Json::Value config_root;
    Json::Reader r;

    if (!r.parse(str_config, config_root))
    {
        return false;
    }

    const char * const TOTAL_MYSQL_CONFIG_COUNT = "TotalMysqlConfigCount";
    const char * const MYSQL_CONFIG_BASE = "MysqlConfig";

    int total_mysql_config_count = 0;
    SAFE_GET_JSON_INT( config_root, TOTAL_MYSQL_CONFIG_COUNT, total_mysql_config_count );

    _vec_mysql_config.reserve(total_mysql_config_count);

    char json_key[128] = { 0 };
    for (int i = 0; i < total_mysql_config_count; ++i)
    {
        memset(json_key, 0, sizeof(json_key));
        snprintf(json_key, sizeof(json_key),
                "%s%d", 
                MYSQL_CONFIG_BASE, i);

        Json::Value & jsn_node = config_root[json_key];

        MysqlConfig mysql_config;

        if (!parse_mysql_config_impl(jsn_node, mysql_config))
        {
            return false;
        }

        _vec_mysql_config.push_back(mysql_config);
    }

    return true;
}

bool MysqlConfigLoad::parse_mysql_config_impl(Json::Value & jsn_mysql_config,
            MysqlConfig & mysql_config)
{
    SAFE_GET_JSON_STRING( jsn_mysql_config, "Host", mysql_config._mysql_host );
    SAFE_GET_JSON_INT( jsn_mysql_config, "Port", mysql_config._mysql_port );
    SAFE_GET_JSON_STRING( jsn_mysql_config, "DbName", mysql_config._mysql_dbname);
    SAFE_GET_JSON_STRING( jsn_mysql_config, "User", mysql_config._mysql_user);
    SAFE_GET_JSON_STRING( jsn_mysql_config, "Passwd", mysql_config._mysql_passwd );
    SAFE_GET_JSON_INT( jsn_mysql_config, "ConnectionPoolSize", mysql_config._mysql_connection_pool_size);

#undef SAFE_GET_JSON_STRING
#undef SAFE_GET_JSON_INT
    return true;
}

} 
} 

