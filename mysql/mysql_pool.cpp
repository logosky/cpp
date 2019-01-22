#include "db_pch.h"
#include "mysql_pool.h"
#include <mysql/mysql.h>

namespace demo
{
namespace mysql
{

//  由于m_PoolSize m_DBType未使用，从SttDBPool类中删除掉，但这里的参数需要保留
MysqlPool::MysqlPool(DBType dbt, char* strPath, char* strUser, char* strPwd, int PoolSize)
    :  m_TotalCount(0)
{            
    m_Path = strdup(strPath);
    m_User = strdup(strUser);
    m_Pwd = strdup(strPwd);
}

MysqlPool::~MysqlPool(void)
{
    boost::lock_guard<boost::mutex> gate(m_Mutex);

    while (!m_Conns.empty())
    {
        IConnection * pConn = m_Conns.front();
        m_Conns.pop_front();

        pConn->Close();
        delete pConn;
    }

    if (m_Path)
    {
        free(m_Path);
    }
    
    if (m_User)
    {
        free(m_User);
    }

    if (m_Pwd)
    {
        free(m_Pwd);
    }
}

IConnection * MysqlPool::GetConn()
{
    MySQLConnection * conn = new MySQLConnection();
    conn->SetAccessUrl(m_Path);
    conn->SetPass(m_User, m_Pwd);


    conn->Open();
    return conn;
    //boost::lock_guard<boost::mutex> gate(m_Mutex);

    //if (!m_Conns.empty())
    //{
    //  IConnection* pConn = m_Conns.front();
    //  m_Conns.pop_front();

    //  //m_Mutex.unlock();
    //  return pConn;
    //}

    //if (m_DBType == dbt_MySQL)
    //{
    //  MySQLConnection * conn = new MySQLConnection();
    //  conn->SetAccessUrl(m_Path);
    //  conn->SetPass(m_User, m_Pwd);

    //  m_Mutex.unlock();
    //  Bool bRet = conn->Open();
    //  m_Mutex.lock();

    //  if (bRet)
    //  {
    //      m_TotalCount++;
    //      //m_Mutex.unlock();
    //      return conn;
    //  }
    //}

    ////m_Mutex.unlock();
    //return NULL;
}

IConnection * MysqlPool::GetConn(unsigned int read_timeout)
{
    MySQLConnection * conn = new MySQLConnection();
    conn->SetAccessUrl(m_Path);
    conn->SetPass(m_User, m_Pwd);
    conn->SetReadTimeout(read_timeout);

    conn->Open();
    return conn;
}

IConnection *MysqlPool::GetConnUseful()
{
    MySQLConnection * conn = new MySQLConnection();
    conn->SetAccessUrl(m_Path);
    conn->SetPass(m_User, m_Pwd);

    if(!conn->Open())            
    {
        CloseConn(conn);
        conn = NULL;
    }

    return conn;
}

IConnection *MysqlPool::GetConnUseful(unsigned int read_timeout)
{
    MySQLConnection * conn = new MySQLConnection();
    conn->SetAccessUrl(m_Path);
    conn->SetPass(m_User, m_Pwd);
    conn->SetReadTimeout(read_timeout);

    if(!conn->Open())            
    {
        CloseConn(conn);
        conn = NULL;
    }

    return conn;
}

const int MysqlPool::GetTotalCount()
{
    return m_TotalCount;
}

void MysqlPool::CloseConn(IConnection * pConn)
{
    if(pConn)
    {
        pConn->Close();
        delete pConn;
    }            
    mysql_thread_end();
    //boost::lock_guard<boost::mutex> gate(m_Mutex);

    //if(m_TotalCount > m_PoolSize)
    //{
    //  --m_TotalCount;
    //  pConn->Close();
    //  delete pConn;
    //  return;
    //}

    //m_Conns.push_front(pConn);
    //return;
}

}
}
