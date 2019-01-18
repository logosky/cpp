#include "lua_script_manager.h"

#include <fstream>
#include <sstream>

namespace demo
{

LuaScriptManager::LuaScriptManager(const std::string & script_dir, const ScriptItem * script_items, int script_item_size, int max_script_id) : _script_dir(script_dir)
{
    _script_items = script_items;
    _script_items_size = script_item_size;
    _max_script_id = (max_script_id > MAX_SCRIPT_ID ? MAX_SCRIPT_ID : max_script_id);

    _lua_script_content_list.resize(_max_script_id + 1);
    _lua_script_sha_list.resize(_max_script_id + 1);

    LOG_PRINTF("script_item_size:%d max_script_id:%d", _script_items_size, _max_script_id);
}

LuaScriptManager::~LuaScriptManager()
{
}

bool LuaScriptManager::load_script_file()
{
    for (int i = 0; i < _script_items_size; i++)
    {
        const ScriptItem * item = &_script_items[i];
        if (item->_name.empty() || item->_id < 0 || item->_id > _max_script_id)
        {
            LOG_PRINTF("invalid script item, max_id:%d, id:%d, name:%s", _max_script_id, item->_id, item->_name.c_str());
            continue;
        }

        std::string script_content;
        if(!get_file_content(item->_name, script_content))
        {
            return false;
        }

        // 更新脚本内容
        set_lua_script_content(item->_id, script_content);

        // 支持reload热更新，清空sha1值
        set_lua_script_sha(item->_id, "");

        LOG_PRINTF("load script funished, id:%d name:%s", item->_id, item->_name.c_str());
    }

    return true;
}

bool LuaScriptManager::get_file_content(const std::string & script_name, std::string & script_content)
{
    std::string script_path = _script_dir + "/" + script_name;
    std::ifstream fs(script_path.c_str(), std::ios::binary); 

    if(!fs.is_open())
    {
        LOG_PRINTF(0, "load script file failed. read script file[%s] failed.", script_path.c_str());
        fs.close();
        return false;
    }

    std::stringstream ss_script_content;
    ss_script_content << fs.rdbuf();

    script_content = ss_script_content.str();

    LOG_PRINTF("script name:%s content:%s", script_name.c_str(), script_content.c_str());

    fs.close();

    return true;
}

const char * LuaScriptManager::script_name(int id)
{
    for (int i = 0; i < _script_items_size; i++)
    {
        const ScriptItem * item = &_script_items[i];
        if (item->_id == id)
        {
            return item->_name.c_str();
        }
    }

    return "";
}

bool LuaScriptManager::load_lua_script_to_redis(demo::hiredis::Client * redis_client, int lua_no, std::string & script_sha1)
{
    std::string script_content = get_lua_script_content(lua_no);
    if (script_content.empty())
    {
        LOG_PRINTF(0, "lua script empty:%s", script_name(lua_no));
        return false;
    }

    demo::hiredis::RedisScript redis_script(*redis_client);
    if (!redis_script.Load(script_content, script_sha1))
    {
        LOG_PRINTF(0, "load script to redis failed, err:%s", redis_script.ErrMsg().c_str());
        return false;
    }

    set_lua_script_sha(lua_no, script_sha1);

    LOG_PRINTF("luad lua script to redis success, name:%s sha1:%s", script_name(lua_no), script_sha1.c_str());

    return true;
}

bool LuaScriptManager::exec_lua_script(demo::hiredis::Client * redis_client,
        int lua_no,
        const std::vector<std::string> & keys,
        const std::vector<std::string> & args,
        demo::hiredis::Reply & reply)
{
    if (lua_no > _max_script_id)
    {
        LOG_PRINTF(0, "lua_no exceeds _max_script_id, lua_no:%d, _max_script_id:%d", lua_no, _max_script_id);
        return false;
    }

    std::string script_sha1 = get_lua_script_sha(lua_no);

    // sha1不存在，加载
    if (script_sha1.empty())
    {
        if (!load_lua_script_to_redis(redis_client, lua_no, script_sha1))
        {
            LOG_PRINTF(0, "load script to redis failed");
            return false;
        }
    }

    LOG_PRINTF("get sha1 %s:%s", script_name(lua_no), script_sha1.c_str());

    // 此时sha1已存在，但有可能是从缓存获取的，可能失效
    // 先执行脚本，若报错sha1无效，则再强制加载一遍
    demo::hiredis::RedisScript redis_script(*redis_client);
    if (!redis_script.EvalSha(script_sha1, keys, args, reply))
    {
        if (reply.Type() == demo::hiredis::ERROR && strncmp(reply.Str().c_str(), "NOSCRIPT", strlen("NOSCRIPT")) == 0)
        {
            LOG_PRINTF(0, "script not exist, load script again, name:%s", script_name(lua_no));
            if (!load_lua_script_to_redis(redis_client, lua_no, script_sha1))
            {
                LOG_PRINTF(0, "load script to redis failed");
                return false;
            }

            return redis_script.EvalSha(script_sha1, keys, args, reply);
        }
        else
        {
            LOG_PRINTF(0, "evalsha type:%d error:%s", reply.Type(), reply.Str().c_str());
            return false;
        }
    }

    return true;
}

} // namespace demo
