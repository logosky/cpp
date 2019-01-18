
#ifndef __LUA_SCRIPT_MANAGER__H__
#define __LUA_SCRIPT_MANAGER__H__

#include <string>
#include <vector>

#include "hiredisclient.h"
#include "utils.h"

namespace demo
{

const static int MAX_SCRIPT_ID = 1000;

struct ScriptItem
{
    int _id;
    std::string _name;
};

class LuaScriptManager
{
public:
    /**
     * @param script_item 存储脚本id和名字的数组
     * @param script_item_size script_item数组元素个数
     * @param max_script_id script_item数组中id最大值
     */
    LuaScriptManager(const std::string & script_dir, const ScriptItem * script_item, int script_item_size, int max_script_id);
    ~LuaScriptManager();

public:
    // 加载lua脚本，可热加载
    bool load_script_file();

    /**
     * @brief 根据脚本id执行脚本
     * @param redis_client hiredis连接客户端
     * @param lua_no 脚本id
     * @param keys 脚本key列表
     * @param args 脚本参数列表
     * @param reply 执行结果
     * @return true: 执行成功， false:执行失败
     */
    bool exec_lua_script(demo::hiredis::Client * redis_client,
            int lua_no,
            const std::vector<std::string> & keys,
            const std::vector<std::string> & args,
            demo::hiredis::Reply & reply);

private:
    /** 
     * @brief 加载脚本到redis，不判断sha1是否存在或有效，强制加载
     * @param redis_conn redis连接对象
     * @param lua_no lua脚本id
     * @param script_sha1 加载成功后redis返回的脚本sha1值
     * @return true: 加载成功，false:加载失败
     */
    bool load_lua_script_to_redis(demo::hiredis::Client * redis_conn, int lua_no, std::string & script_sha1);

    // 根据脚本id返回名字
    const char * script_name(int id);

    const std::string get_lua_script_content(int lua_script_no)
    {
        Auto_Read_Lock r_lock(_content_list_lock);
        return _lua_script_content_list[lua_script_no];
    }

    void set_lua_script_content(int lua_script_no, const std::string & lua_script_content)
    {
        Auto_Write_Lock w_lock(_content_list_lock);
        _lua_script_content_list[lua_script_no] = lua_script_content;
    }

    const std::string get_lua_script_sha(int lua_script_no)
    {
        Auto_Read_Lock r_lock(_sha_list_lock);
        return _lua_script_sha_list[lua_script_no];
    }

    void set_lua_script_sha(int lua_script_no, const std::string & lua_script_sha)
    {
        Auto_Write_Lock w_lock(_sha_list_lock);
        _lua_script_sha_list[lua_script_no] = lua_script_sha;
    }

    bool get_file_content(const std::string & script_name, std::string & script_content);

private:
    std::string _script_dir;
    const ScriptItem * _script_items;
    int _script_items_size;
    int _max_script_id;

    Read_Write_Lock _content_list_lock;
    std::vector<std::string> _lua_script_content_list;
    
    Read_Write_Lock _sha_list_lock;
    std::vector<std::string> _lua_script_sha_list;
};

} 
#endif
