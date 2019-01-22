
#ifndef __MYSQL_CONFIG_H__
#define __MYSQL_CONFIG_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

namespace Json
{
    class Value;
} // namespace Json

namespace demo
{
namespace mysql
{

struct MysqlConfig
{
    MysqlConfig() :
        _mysql_port(0),
        _mysql_connection_pool_size(0)
    {}

    std::string to_string() const
    {
        std::stringstream ss;
        ss<<"host:"<<_mysql_host<<" "
            <<"port:"<<_mysql_port<< " "
            <<"user:"<<_mysql_user<< " "
            <<"passwd:"<<_mysql_passwd<< " "
            <<"db:"<<_mysql_dbname<< " "
            <<"pool_size:"<<_mysql_connection_pool_size;
        
        return ss.str();
    }
    std::string _mysql_host;
    uint16_t    _mysql_port;
    std::string _mysql_dbname;
    std::string _mysql_user;
    std::string _mysql_passwd;
    uint32_t    _mysql_connection_pool_size;
};

class MysqlConfigLoad
{
public:
    MysqlConfigLoad(const char * config_name);
    ~MysqlConfigLoad();

    bool load_config();

    const std::vector<MysqlConfig> & get_mysql_config() const
    {
        return _vec_mysql_config;
    }

private:
    bool parse_config(const char * str_config);

    bool parse_mysql_config_impl(Json::Value & jsn_mysql_config,
            MysqlConfig &  mysql_config);
private:
    std::string _config_name;
    std::vector<MysqlConfig> _vec_mysql_config;

};

} // namespace mysql
}

#endif 

