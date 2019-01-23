
#include <iostream>
#include "mysql_pool.h"
#include "mysql_config.h"
#include "utils.h"
#include "mysql_long_pool.h"
#include <boost/thread.hpp>

using namespace std;

const char* CONFIG_NAME = "mysql_config.json";

demo::mysql::MysqlPool * db_pool;

demo::mysql::MysqlLongPool * db_long_pool;

static volatile bool s_quit = false;

enum PoolType
{
    PT_SHORT = 0,
    PT_LONG = 1,
};

static void signal_int_handler(int /*sig*/)
{
    LOG_PRINTF("get signal");
    
    s_quit = true;
}


int exe_short_sql()
{
    demo::mysql::MySQLConnection* mysql_conn = static_cast<demo::mysql::MySQLConnection*>(db_pool->GetConnUseful(3));
    std::stringstream ss_sql;
    char error[1024] = { 0 };
    if (!mysql_conn)
    {
        LOG_PRINTF("mysql connect failed");

        return -1;
    }
    demo::mysql::MySQLCommand* cmd = NULL;

    do
    {
        ss_sql << "select settle_id from test.settle_status order by settle_id desc limit 1;";
        LOG_PRINTF("sql: %s", ss_sql.str().c_str());
        
        demo::mysql::MySQLRecordset record_set;
        cmd = (demo::mysql::MySQLCommand*)mysql_conn->CreateCommand(ss_sql.str().c_str());
        if (!cmd)
        {
            mysql_conn->GetLastErrorMessage(error, sizeof(error));
            LOG_PRINTF("mysql CreateCommand failed: %s", error);
            return -1;
        }

        if (!cmd->ExecuteQuery(record_set))
        {
            mysql_conn->GetLastErrorMessage(error, sizeof(error));
            LOG_PRINTF("mysql ExecuteQuery failed: %s", error);
            return -1;
        }

        if(record_set.MoveNext())
        {
            int settle_id = record_set.AsInt32("settle_id");
            cout<<"settle_id:"<< settle_id<<endl;
        }
        else
        {
            cout<<"no result"<<endl;
        }
    } while (0);

    if (cmd)
    {
        delete cmd;
        cmd = NULL;
    }
    
    if (mysql_conn)
    {
        db_pool->CloseConn(mysql_conn);
        mysql_conn = NULL;
    }

    return 0;
}

int exe_long_sql()
{
    demo::ConnectionGuard< demo::mysql::MySQLConnection > conn_guard;
    db_long_pool->get_connection(&conn_guard, 200);
    
    demo::mysql::MySQLConnection* conn = conn_guard.get();
    
    std::stringstream ss_sql;
    char error[1024] = { 0 };
    if (!conn)
    {
        LOG_PRINTF("mysql connect failed");

        return -1;
    }
    demo::mysql::MySQLCommand* cmd = NULL;

    do
    {
        ss_sql << "select settle_id from test.settle_status order by settle_id desc limit 1;";
        LOG_PRINTF("sql: %s", ss_sql.str().c_str());
        
        demo::mysql::MySQLRecordset record_set;
        cmd = (demo::mysql::MySQLCommand*)conn->CreateCommand(ss_sql.str().c_str());
        if (!cmd)
        {
            conn->GetLastErrorMessage(error, sizeof(error));
            LOG_PRINTF("mysql CreateCommand failed: %s", error);
            return -1;
        }

        if (!cmd->ExecuteQuery(record_set))
        {
            conn->GetLastErrorMessage(error, sizeof(error));
            LOG_PRINTF("mysql ExecuteQuery failed: %s", error);
            return -1;
        }

        if(record_set.MoveNext())
        {
            int settle_id = record_set.AsInt32("settle_id");
            cout<<"settle_id:"<< settle_id<<endl;
        }
        else
        {
            cout<<"no result"<<endl;
        }
    } while (0);

    if (cmd)
    {
        delete cmd;
        cmd = NULL;
    }

    return 0;
}

int exe_sql(PoolType pool_type)
{
    switch (pool_type)
    {
        case PT_LONG:
            exe_long_sql();
            break;
        case PT_SHORT:
            exe_short_sql();
            break;
        default:
        break;
    }
}


void mysql_fun(PoolType pool_type)
{
    char thread_name[32] = {0};
    snprintf(thread_name, 32, "thread_msql:%d", pool_type);
    pthread_setname_np(pthread_self(), thread_name);
    while(1)
    {
        LOG_PRINTF("pool type:%d", pool_type);

        exe_sql(pool_type);
        
        usleep(100000);
    }
}


int main()
{    
    daemon_init_stdout_open();

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, signal_int_handler);
    signal(SIGTERM, signal_int_handler);

    demo::mysql::MysqlConfigLoad* config_load = new demo::mysql::MysqlConfigLoad(CONFIG_NAME);
    if(!config_load->load_config())
    {
        cout<<"load_config fail."<<endl;
        return 0;
    }
    
    demo::mysql::MySQLConnection::MySQL_Init();

    const std::vector<demo::mysql::MysqlConfig>& configs = config_load->get_mysql_config();
    const demo::mysql::MysqlConfig& config = configs[0];
    cout<<"config:"<<config.to_string()<<endl;
    
    std::stringstream ss;
    ss<<config._mysql_host<<":"<<config._mysql_port;

    db_pool = new demo::mysql::MysqlPool(demo::mysql::dbt_MySQL, 
    const_cast<char*>(ss.str().c_str()),
    const_cast<char*>(config._mysql_user.c_str()),
    const_cast<char*>(config._mysql_passwd.c_str()),
    config._mysql_connection_pool_size);
    
    db_long_pool = new demo::mysql::MysqlLongPool(const_cast<char*>(ss.str().c_str()),
        const_cast<char*>(config._mysql_user.c_str()),
        const_cast<char*>(config._mysql_passwd.c_str()),
        config._mysql_connection_pool_size);

    db_long_pool->init();

    exe_sql(PT_LONG);
    exe_sql(PT_SHORT);
    
/*
    boost::thread* thread_a = new boost::thread(boost::bind(&mysql_fun, PT_LONG));
    boost::thread* thread_b = new boost::thread(boost::bind(&mysql_fun, PT_LONG));
    boost::thread* thread_c = new boost::thread(boost::bind(&mysql_fun, PT_LONG));
    boost::thread* thread_d = new boost::thread(boost::bind(&mysql_fun, PT_SHORT));
*/
    pthread_setname_np(pthread_self(), "main");

    while (!s_quit)
    {
        sleep(1);
    }
    demo::mysql::MySQLConnection::MySQL_End();

    return 0;
}
