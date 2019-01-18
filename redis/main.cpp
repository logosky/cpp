#include <hiredis/hiredis.h>
#include <iostream>
#include "hiredis_pool_group.h"
#include "lua_script_manager.h"

using namespace std;
const char* REDIS_CONFIG_FILE = "redis_config.json";
const char* LUA_DIR = "./script";

enum LuaId
{
    LUA_SET_STRING = 0,
    LUA_SET_ZSET = 1,
    LUA_MAX,
};

struct demo::ScriptItem lua_scripts[] = 
{
    {LUA_SET_STRING, "set_string.lua"},
    {LUA_SET_ZSET, "set_zset.lua"},
};

int main()
{
    demo::RedisConfigLoad * redis_config_loader;
    redis_config_loader = new demo::RedisConfigLoad(REDIS_CONFIG_FILE);
    if(!redis_config_loader->load_config())
    {
        cout<<"redis_config_loader load_config() failed"<<endl;
        return -1;
    }
    
    demo::HiredisPoolGroup * redis_pool_group;
    redis_pool_group = new demo::HiredisPoolGroup(redis_config_loader->get_thread_count(), 
        redis_config_loader->get_redis_config());
    if (!redis_pool_group->init())
    {
        cout<<"redis pool group init failed"<<endl;
        return -1;
    }
    
    demo::ConnectionGuard<demo::hiredis::Client> redis_conn;
    if (!redis_pool_group->get_connection(&redis_conn, 0, 100))
    {
        cout<<"redis pool group get connection failed"<<endl;
        return -1;
    }

    demo::hiredis::RedisString redis_string(*(redis_conn.get()));
    redis_string.Set("test_key", "test_value");
    string value = redis_string.Get("test_key");
    
    cout<<"get value:"<< value<<endl;

    // lua script manager
    demo::LuaScriptManager* lua_manager = new demo::LuaScriptManager(LUA_DIR, 
                                                        lua_scripts, 
                                                        sizeof(lua_scripts)/sizeof(demo::ScriptItem), 
                                                        LUA_MAX);
    if(!lua_manager->load_script_file())
    {
        cout<<"lua_manager->load_script_file failed"<<endl;
        return -1;
    }

    std::vector<std::string> keys;
    std::vector<std::string> args;
    args.push_back("100");
    args.push_back("200");

    demo::hiredis::Reply reply;
    if(!lua_manager->exec_lua_script(redis_conn.get(), 
                                LUA_SET_STRING, 
                                keys, 
                                args, 
                                reply))
    {
        cout<<"lua_manager->exec_lua_script failed"<<endl;
        return -1;
    }
    LOG_PRINTF("reply type:%d\n%s", reply.Type(), reply.to_string().c_str());
    
    return 0;
}
