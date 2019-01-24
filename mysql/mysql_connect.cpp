#include "db_pch.h"
#include "mysql_connect.h"
#include "mysql_helper.h"
#include "utils.h"

#include <inttypes.h>

#ifndef OUTLOG_F
#define OUTLOG_F(x) __noop
#endif

int g_record_sql = 0;

namespace demo
{
namespace mysql
{


MySQLConnection::MySQLConnection()
{
    strcpy(mstrUsername, "");
    strcpy(mstrPassword, "");
    strcpy(mstrAccessUrl, "");
    strcpy(m_last_sys_error, "");
    strcpy(m_last_sql_error, "");
    m_Port = 0;
    m_ReadTimeout = 0;
}

Bool MySQLConnection::SetPassword(const char* strPwd)
{
    strcpy(this->mstrPassword, strPwd);
    return TRUE;
}

void MySQLConnection::SetReadTimeout(unsigned int read_timeout)
{
    if (0 != read_timeout)
    {
        m_ReadTimeout = read_timeout;
    }
}

Bool MySQLConnection::SetAccessUrl(const char* strPath)
{
    char sUrl[256];
    strncpy(sUrl, strPath, sizeof(sUrl));
    char * sPort = strstr(sUrl, ":");

    if (sPort == NULL)
    {
        m_Port = 0;
    }
    else
    {
        sPort[0] = '\0';
        sPort++;
        m_Port = atoi(sPort);
    }

    strcpy(this->mstrAccessUrl, sUrl);
    return TRUE;
}
Bool MySQLConnection::SetPass(const char* strUser, const char* strPwd)
{
    strcpy(this->mstrUsername, strUser);
    strcpy(this->mstrPassword, strPwd);
    return TRUE;
}

Bool MySQLConnection::Open()
{
    std::string strServer = (const char *)mstrAccessUrl;
    std::string strUsername = (const char *)mstrUsername;
    std::string strPassword = (const char *)mstrPassword;

    if (mysql_init(&m_conn) == NULL)
    {
        LOG_PRINTF("init error %d.message:%s", mysql_errno(&m_conn), mysql_error(&m_conn));
        return FALSE;
    }

    // 设置读取的超时时间
    if (0 != m_ReadTimeout)
    {
        mysql_options(&m_conn, MYSQL_OPT_READ_TIMEOUT, (unsigned int *)&m_ReadTimeout);
    }

    my_bool bReconnect = true;
    mysql_options(&m_conn, MYSQL_OPT_RECONNECT, (char *)&bReconnect);

    if (mysql_real_connect(&m_conn, strServer.c_str(), strUsername.c_str(), strPassword.c_str(), NULL, m_Port, NULL,
                           CLIENT_FOUND_ROWS | CLIENT_MULTI_STATEMENTS) == NULL)
    {
        //OUTLOG_F((xc, "connect error %d.message:%s", mysql_errno(&m_conn),mysql_error(&m_conn)));
       LOG_PRINTF("connect error %d.message:%s", mysql_errno(&m_conn), mysql_error(&m_conn));
        return FALSE;
    }

    mysql_set_character_set(&m_conn, "utf8");
    return TRUE;
}

Bool MySQLConnection::Close()
{
    mysql_close(&m_conn);
    return TRUE;
}

Int MySQLConnection::Ping()
{
    return mysql_ping(&m_conn);
}

long MySQLConnection::BeginTrans()
{
    const char * _sql = "START TRANSACTION";
    return mysql_real_query(&m_conn, _sql, strlen(_sql));
}


Bool MySQLConnection::RollbackTrans()
{
    const char * _sql = "ROLLBACK";
    return mysql_real_query(&m_conn, _sql, strlen(_sql)) == 0;
}

Bool MySQLConnection::CommitTrans()
{
    const char * _sql = "COMMIT";
    return mysql_real_query(&m_conn, _sql, strlen(_sql)) == 0;
}

char* MySQLConnection::EscapeString(const char * input, size_t input_len, char * output)
{
    assert(input);
    assert(output);

    mysql_real_escape_string(&m_conn, output, input, input_len);
    return output;
}

UInt64 MySQLConnection::LastInsertID(Bool * Err /*= NULL*/)
{
    return (UInt64)mysql_insert_id(&m_conn);
}


//错误码
UInt32 MySQLConnection::GetLastError()
{
    return mysql_errno(&m_conn);
}
//错误信息
char* MySQLConnection::GetLastErrorMessage(char* strBuf, Int MaxSize)
{
    const char * msg = mysql_error(&m_conn);

    if (msg)
    {
        strncpy(strBuf, msg, MaxSize);
        return strBuf;
    }

    return NULL;
}

const char* MySQLConnection::GetLastErrorMessage()
{
    size_t max_error_size = sizeof(m_last_sys_error);
    const char * msg = mysql_error(&m_conn);

    if (msg)
    {
        strncpy(m_last_sys_error, msg, max_error_size - 1);
    }

    return m_last_sys_error;
}
//SHOW ERRORS返回的错误
char* MySQLConnection::GetSQLError(char* strBuf, Int MaxSize)
{
    strBuf[0] = 0;
    const char * _sql = "SHOW ERRORS";

    if (mysql_real_query(&m_conn, _sql, strlen(_sql)))
    {
        return NULL;
    }

    MYSQL_RES * pRes = mysql_store_result(&m_conn);

    if (pRes)
    {
        MYSQL_ROW Row;
        char tmp[512];
        int i = 0;
        sprintf(strBuf, "SHOW ERRORS:\n");

        while ((Row = mysql_fetch_row(pRes)))
        {
            i++;
            sprintf(tmp, _T("%d %s(%d)\n"), i, Row[0], *(int *)Row[1]);

            if ((int)(strlen(strBuf) + strlen(tmp)) >= MaxSize)
            {
                break;
            }

            strcat(strBuf, tmp);
        }
    }

    return strBuf;
}

const char* MySQLConnection::GetSQLError()
{
    size_t max_sql_error_size = sizeof(m_last_sql_error);
	
	memset(m_last_sql_error, 0, max_sql_error_size);
    
    const char * _sql = "SHOW ERRORS";

    // 执行失败
    if (0 != mysql_real_query(&m_conn, _sql, strlen(_sql)))
    {
        return m_last_sys_error;
    }

    // 执行成功
    MYSQL_RES * mysql_res = mysql_store_result(&m_conn);
    if (mysql_res)
    {
        MYSQL_ROW Row;
        char tmp[512];
        int i = 0;
        sprintf(m_last_sql_error, "SHOW ERRORS:\n");

        while ((Row = mysql_fetch_row(mysql_res)))
        {
            i++;
            sprintf(tmp, _T("%d %s(%d)\n"), i, Row[0], *(int *)Row[1]);

            if ((int)(strlen(m_last_sql_error) + strlen(tmp)) >= max_sql_error_size - 1)
            {
                break;
            }

            strcat(m_last_sys_error, tmp);
        }
    }    
        
    return m_last_sys_error;
}

ICommand * MySQLConnection::CreateCommand(const char* CmdTxt /* = NULL*/)
{
    MySQLCommand * pCommand = new MySQLCommand(*this);

    if (pCommand)
    {
        if (CmdTxt == NULL)
        {
            return pCommand;
        }

        if (pCommand->PrepareCommand(CmdTxt))
        {
            return (ICommand *)pCommand;
        }
        else
        {
            delete pCommand;
            //assert(0);
            return NULL;
        }
    }
    else
    {
        assert(0);
        return NULL;
    }
}

MySQLConnection::~MySQLConnection()
{
    //Close();
}

void MySQLConnection::MySQL_Init()
{
    mysql_library_init(0, NULL, NULL);
}

void MySQLConnection::MySQL_End()
{
    mysql_library_end();
}


//////////////////////////////////////////////////////////////////////////
boost::detail::atomic_count   MySQLCommand::exec_cmd_count(0);

MySQLCommand::MySQLCommand(MySQLConnection & conn)
{
    m_nParamterAppended = 0;
    m_Params = NULL;
    m_nParamCount = 0;
    m_Statement = NULL;
    m_Connection = &conn;
    m_Cmd = NULL;
}

Bool MySQLCommand::PrepareCommand(const char* CmdTxt)
{
    m_Statement = mysql_stmt_init(&m_Connection->m_conn);

    if (m_Statement == NULL)
    {
        return FALSE;
    }

    if (mysql_stmt_prepare(m_Statement, CmdTxt, strlen(CmdTxt)))
    {
        LOG_PRINTF("stmt_init error %d message:%s", 
            mysql_errno(&m_Connection->m_conn), 
            mysql_error(&m_Connection->m_conn));
        return FALSE;
    }

    m_nParamCount = mysql_stmt_param_count(m_Statement);

    if (m_nParamCount > 0)
    {
        m_Params = new MYSQL_BIND[m_nParamCount];
        memset(m_Params, 0, sizeof(MYSQL_BIND)*m_nParamCount);
    }

    if (CmdTxt)
    {
        m_Cmd = new char[strlen(CmdTxt) + 1];

        if (m_Cmd)
        {
            strcpy(m_Cmd, CmdTxt);
        }
    }

    return TRUE;
}
Int MySQLCommand::ExecuteNoQuery()
{
    ++exec_cmd_count;
    uint64_t tickStart = get_tick_count();
    // 发起mysql调用
    
    my_bool ret = mysql_stmt_bind_param(m_Statement, m_Params);

    if (ret)
    {
        return -1;
    }

    int nRet = mysql_stmt_execute(m_Statement);

    if (nRet)
    {
        LOG_PRINTF("MySQLCommand::ExecuteNoQuery() faild cmd:%s, message:%d,%s", m_Cmd,
                    mysql_errno(&m_Connection->m_conn), mysql_error(&m_Connection->m_conn));

        return -1;
    }

    uint64_t tickEscape = get_tick_count() - tickStart;

    if (tickEscape > 500 && m_Cmd)
    {
        LOG_PRINTF("MySQLCommand::ExecuteNoQuery() cmd:%s time:%d", m_Cmd, tickEscape);
    }

    if (g_record_sql)
    {
        LOG_PRINTF("cmd:%s,time:%" PRId64 , m_Cmd, tickEscape);
    }

    return (Int)mysql_affected_rows(&m_Connection->m_conn);
}

IRecordset * MySQLCommand::ExecuteQuery(IRecordset & rs)
{
    ++exec_cmd_count;
    uint64_t tickStart = get_tick_count();
    
    // 发起mysql调用

    my_bool ret = mysql_stmt_bind_param(m_Statement, m_Params);
    if (ret)
    {
        return NULL;
    }

    int nRet = mysql_stmt_execute(m_Statement);

    if (nRet)
    {
        LOG_PRINTF("MySQLCommand::ExecuteQuery() faild cmd:%s, message:%d,%s", m_Cmd,
                    mysql_errno(&m_Connection->m_conn), mysql_error(&m_Connection->m_conn));
        
        return NULL;
    }

    if (((MySQLRecordset &)rs).BindResult(this))
    {
    }
    else
    {
        nRet = 1;
    }

    uint64_t tickEscape = get_tick_count() - tickStart;

    if (tickEscape > 500 && m_Cmd)
    {
        LOG_PRINTF("MySQLCommand::ExecuteQuery() cmd:%s time:%d", m_Cmd, tickEscape);
    }

    if (g_record_sql)
    {
        LOG_PRINTF("cmd:%s,time:%" PRId64 , m_Cmd, tickEscape);
    }

    return nRet == 0 ? &rs : NULL;
}

IRecordset * MySQLCommand::ExecuteQueryNoSTMT(const char* CmdTxt, IRecordset & rs)
{
    ++exec_cmd_count;
    uint64_t tickStart = get_tick_count();
    // 发起mysql调用
    int nRet = mysql_real_query(&m_Connection->m_conn, CmdTxt, strlen(CmdTxt));

    if (nRet)
    {
        LOG_PRINTF("MySQLCommand::ExecuteQueryNoSTMT() faild cmd:%s, message:%d,%s", CmdTxt,
                    mysql_errno(&m_Connection->m_conn), mysql_error(&m_Connection->m_conn));

        return NULL;
    }

    if (((NoSTMTSQLRecordset &)rs).BindResult(&m_Connection->m_conn))
    {
    }
    else
    {
        nRet = 1;
    }

    uint64_t tickEscape = get_tick_count() - tickStart;

    if (tickEscape > 500 && m_Cmd)
    {
        LOG_PRINTF("MySQLCommand::ExecuteQueryNoSTMT() cmd:%s time:%d", CmdTxt, tickEscape);
    }

    if (g_record_sql)
    {
        LOG_PRINTF("cmd:%s,time:%" PRId64 , CmdTxt, tickEscape);
    }

    return nRet == 0 ? &rs : NULL;
}

Int MySQLCommand::ExecuteNoQuery(const char* CmdTxt)
{
    ++exec_cmd_count;
    uint64_t tickStart = get_tick_count();
    // 发起mysql调用

    int nRet = mysql_real_query(&m_Connection->m_conn, CmdTxt, strlen(CmdTxt));

    if (nRet)
    {
        LOG_PRINTF("MySQLCommand::ExecuteNoQuery() faild cmd:%s, message:%d,%s", CmdTxt,
                    mysql_errno(&m_Connection->m_conn), mysql_error(&m_Connection->m_conn));

        return -1;
    }

    uint64_t tickEscape = get_tick_count() - tickStart;

    if (tickEscape > 500 && CmdTxt)
    {
        LOG_PRINTF("MySQLCommand::ExecuteNoQuery() cmd:%s time:%d", CmdTxt, tickEscape);
    }

    if (g_record_sql)
    {
        LOG_PRINTF("cmd:%s,time:%" PRId64 , CmdTxt, tickEscape);
    }

    return (int)mysql_affected_rows(&m_Connection->m_conn);
}

Bool MySQLCommand::AppendParameter(MySQLParameter & prm)
{
    if (m_nParamterAppended >= m_nParamCount)
    {
        return FALSE;
    }

    if (ptInt8 == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_TINY;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new Int8(prm.i8val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 0;
        delete param->length;
        param->length = NULL;
    }
    else if (ptUInt8 == prm.mDataType || ptByte == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_TINY;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new UInt8(prm.ui8val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 1;
        delete param->length;
        param->length = NULL;
    }
    else if (ptInt16 == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_SHORT;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new Int16(prm.i16val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 0;
        delete param->length;
        param->length = NULL;
    }
    else if (ptUInt16 == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_SHORT;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new UInt16(prm.ui16val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 1;
        delete param->length;
        param->length = NULL;
    }
    else if (ptInt32 == prm.mDataType || ptInt == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_LONG;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new Int32(prm.i32val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 0;
        delete param->length;
        param->length = NULL;
    }
    else if (ptUInt32 == prm.mDataType || ptUInt == prm.mDataType || ptDBKey == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_LONG;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new UInt32(prm.ui32val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 1;
        delete param->length;
        param->length = NULL;
    }
    else if (ptLong == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_LONG;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new Int32(prm.lval);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 0;
        delete param->length;
        param->length = NULL;
    }
    else if (ptULong == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_LONG;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new UInt32(prm.ulval);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 1;
        delete param->length;
        param->length = NULL;
    }
    else if (ptInt64 == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_LONGLONG;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new Int64(prm.i64val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 0;
        delete param->length;
        param->length = NULL;
    }
    else if (ptUInt64 == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_LONGLONG;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new UInt64(prm.ui64val);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 1;
        delete param->length;
        param->length = NULL;
    }
    else if (ptDouble == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_DOUBLE;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new DOUBLE(prm.dval);
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->is_unsigned = 0;
        delete param->length;
        param->length       = NULL;
    }
    else if (ptDate == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_DATETIME;
        delete [] static_cast<char *>(param->buffer);
        MYSQL_TIME * dt = new MYSQL_TIME;
        param->buffer = dt;
        param->buffer_length = 0;
        param->is_null_value = 0;
        delete param->length;
        param->length       = NULL;
        dt->year = prm.dtval.Year;
        dt->month = prm.dtval.Month;
        dt->day = prm.dtval.Day;
        dt->hour = prm.dtval.Hour;
        dt->minute = prm.dtval.Minute;
        dt->second = prm.dtval.Second;
    }
    else if (ptString == prm.mDataType || ptVarChar == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_STRING;
        delete [] static_cast<char *>(param->buffer);
        int strLen = strlen(prm.strval);
        param->buffer = new char[sizeof(char) * (strLen + 1)];
        memcpy(param->buffer, prm.strval, sizeof(char) * (strLen + 1));
        param->buffer_length = sizeof(char) * (strLen + 1);
        param->is_null_value = 0;
        delete param->length;
        param->length = new unsigned long(static_cast<unsigned long>(sizeof(char)*strLen));
    }
    else if (ptBinary == prm.mDataType)
    {
        MYSQL_BIND * param = &m_Params[m_nParamterAppended];
        param->buffer_type = MYSQL_TYPE_BLOB;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new char[prm.mSize];
        memcpy(param->buffer, prm.pBlob, prm.mSize);
        param->buffer_length = prm.mSize;
        param->is_null_value = 0;
        delete param->length;
        param->length = new unsigned long(static_cast<unsigned long>(prm.mSize));
    }

    m_nParamterAppended++;
    return TRUE;
}

Bool MySQLCommand::AppendParameter(MySQLParameterset & ps)
{
    for (Int i = 0; i < ps.GetSize(); i ++)
    {
        this->AppendParameter(ps.Get(i));
    }

    return TRUE;
}

Bool MySQLCommand::AppendParameter(const Int8 i)
{
    return this->AppendParameter(MySQLParameter(i));
}

Bool MySQLCommand::AppendParameter(const UInt8 i)
{
    return this->AppendParameter(MySQLParameter(i));
}

Bool MySQLCommand::AppendParameter(const Int16 i)
{
    return this->AppendParameter(MySQLParameter(i));
}

Bool MySQLCommand::AppendParameter(const UInt16 i)
{
    return this->AppendParameter(MySQLParameter(i));
}

Bool MySQLCommand::AppendParameter(const Int32 i)
{
    return this->AppendParameter(MySQLParameter(i));
}

Bool MySQLCommand::AppendParameter(const UInt32 i)
{
    return this->AppendParameter(MySQLParameter(i));
}

/*    Bool MySQLCommand::AppendParameter(const Long l)
    {
        return this->AppendParameter(MySQLParameter(l));
    }
    Bool MySQLCommand::AppendParameter(const ULong l)
    {
        return this->AppendParameter(MySQLParameter(l));
    }
    */
Bool MySQLCommand::AppendParameter(const Int64 ll)
{
    return this->AppendParameter(MySQLParameter(ll));
}
Bool MySQLCommand::AppendParameter(const UInt64 ll)
{
    return this->AppendParameter(MySQLParameter(ll));
}
Bool MySQLCommand::AppendParameter(const Double d, ParameterType pt)
{
    return this->AppendParameter(MySQLParameter(d, pt));
}
Bool MySQLCommand::AppendParameter(const DBDateTime & dt)
{
    return this->AppendParameter(MySQLParameter(dt));
}
Bool MySQLCommand::AppendParameter(char* str)
{
    return this->AppendParameter(MySQLParameter(str));
}
Bool MySQLCommand::AppendParameter(const Byte blob[], Int size)
{
    return this->AppendParameter(MySQLParameter(blob, size));
}

Bool MySQLCommand::AppendParameter(const  IParameter & prm)
{
    return this->AppendParameter((MySQLParameter &)prm);
}

Bool MySQLCommand::AppendParameter(const  IParameterset & ps)
{
    return this->AppendParameter((MySQLParameterset &)ps);
}

MySQLCommand::~MySQLCommand(void)
{
    if (m_Statement)
    {
        do 
        {
            mysql_stmt_free_result(m_Statement);
        } while (!mysql_next_result(&m_Connection->m_conn));
        
        mysql_stmt_close(m_Statement);
    }

    if (m_Params && m_nParamCount > 0)
    {
        for (int i = 0; i < m_nParamCount; i++)
        {
            if (m_Params[i].buffer_type == MYSQL_TYPE_STRING || m_Params[i].buffer_type == MYSQL_TYPE_BLOB)
            {
                delete [](char *)m_Params[i].buffer;
            }
            else
            {
                delete(char *)m_Params[i].buffer;
            }

            delete(char *)m_Params[i].length;
        }

        delete [] m_Params;
    }

    if (m_Cmd)
    {
        delete [] m_Cmd;
    }
}

//////////////////////////////////////////////////////////////////////////

MySQLRecordset::MySQLRecordset()
{
    m_ResultSetMetadata = NULL;
    m_ResultRow = NULL;
    m_DataLen = NULL;
    m_IsNull = NULL;
    m_Err = NULL;
}

MySQLRecordset::~MySQLRecordset()
{
    if (m_ResultSetMetadata)
    {
        mysql_free_result(m_ResultSetMetadata);
    }

    if (m_FieldNum > 0 && m_ResultRow)
    {
        for (int i = 0; i < m_FieldNum; i++)
        {
            delete [](char *)m_ResultRow[i].buffer;
        }

        delete [] m_ResultRow;
        delete [] m_DataLen;
        delete [] m_IsNull;
        delete [] m_Err;
    }
}

bool MySQLRecordset::BindResult(MySQLCommand * command)
{
    //绑定结果
    int nRet = 0;
    m_Command = command;
    m_ResultSetMetadata = mysql_stmt_result_metadata(m_Command->m_Statement);

    if (m_ResultSetMetadata)
    {
        nRet = mysql_stmt_store_result(m_Command->m_Statement);

        if (nRet == 0)
        {
            m_Fields = mysql_fetch_fields(m_ResultSetMetadata);
            m_FieldNum = mysql_num_fields(m_ResultSetMetadata);

            if (m_FieldNum > 0 && m_Fields)
            {
                m_ResultRow = new MYSQL_BIND[m_FieldNum];
                memset(m_ResultRow, 0, m_FieldNum * sizeof(MYSQL_BIND));

                m_DataLen = new unsigned long[m_FieldNum];
                memset(m_DataLen, 0, m_FieldNum * sizeof(unsigned long));

                m_IsNull = new char[m_FieldNum];
                memset(m_IsNull, 0, m_FieldNum * sizeof(char));

                m_Err = new char[m_FieldNum];
                memset(m_Err, 0, m_FieldNum * sizeof(char));

                for (int i = 0; i < m_FieldNum; ++i)
                {
                    struct st_buffer_size_type pFieldData = allocate_buffer_for_field(&m_Fields[i]);
                    m_ResultRow[i].buffer_type = pFieldData.type;
                    m_ResultRow[i].buffer       = pFieldData.buffer;
                    m_ResultRow[i].buffer_length = static_cast<unsigned long>(pFieldData.size);
                    m_ResultRow[i].length       = &m_DataLen[i];
                    m_ResultRow[i].is_null      = &m_IsNull[i];
                    m_ResultRow[i].error        = &m_Err[i];
                    m_ResultRow[i].is_unsigned = m_Fields[i].flags & UNSIGNED_FLAG;
                }

                mysql_stmt_bind_result(m_Command->m_Statement, m_ResultRow);
                m_num_rows = (int)mysql_stmt_num_rows(m_Command->m_Statement);
                m_row_position = 0;
            }
        }
    }
    else
    {
        m_Command = NULL;
        nRet = 1;
    }

    return nRet == 0;
}
Bool MySQLRecordset::IsEmpty(Bool * Err)
{
    if (Err)
    {
        *Err = TRUE;
    }

    return (m_num_rows == 0);
}

Bool MySQLRecordset::IsBOF(Bool * Err)
{
    if (Err)
    {
        *Err = TRUE;
    }

    return (m_row_position < 1);
}

Bool MySQLRecordset::IsAOL(Bool * Err)
{
    if (Err)
    {
        *Err = TRUE;
    }

    return (m_row_position > m_num_rows);
}

Bool MySQLRecordset::MoveFirst()
{
    m_row_position = 1;
    mysql_stmt_data_seek(m_Command->m_Statement, m_row_position);
    return TRUE;
}


Bool MySQLRecordset::MoveNext()
{
    m_row_position++;
    int ret = mysql_stmt_fetch(m_Command->m_Statement);

    if (ret)
    {
        if (ret == MYSQL_DATA_TRUNCATED)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

Int8 MySQLRecordset::AsInt8(const char * column, Bool * Err)
{
    Int8 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Int16 MySQLRecordset::AsInt16(const char * column, Bool * Err)
{
    Int16 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Int32 MySQLRecordset::AsInt32(const char * column, Bool * Err)
{
    Int32 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

UInt8 MySQLRecordset::AsUInt8(const char * column, Bool * Err)
{
    UInt8 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

UInt16 MySQLRecordset::AsUInt16(const char * column, Bool * Err)
{
    UInt16 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

UInt32 MySQLRecordset::AsUInt32(const char * column, Bool * Err)
{
    UInt32 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
Int64 MySQLRecordset::AsInt64(const char * column, Bool * Err)
{
    Int64 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
UInt64 MySQLRecordset::AsUInt64(const char * column, Bool * Err)
{
    UInt64 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Byte MySQLRecordset::AsByte(const char * column, Bool * Err)
{
    Byte v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Int MySQLRecordset::AsInteger(const char * column, Bool * Err)
{
    Int v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

//Long MySQLRecordset::AsLong1(const char * column,Bool * Err){
//    Long v;
//    Bool b = this->GetValue(column,v);
//    if(Err) {*Err = b;if(!b) return 0;};
//    return v;
//}

Float MySQLRecordset::AsFloat(const char * column, Bool * Err)
{
    Float v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
Double  MySQLRecordset::AsDouble(const char * column, Bool * Err)
{
    Double v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
Bool MySQLRecordset::AsDateTime(const char * column, DBDateTime & dt)
{
    return this->GetValue(column, dt);
}

char* MySQLRecordset::AsString(const char * column, char* strBuf, Int MaxLength)
{
    return this->GetValue(column, strBuf, MaxLength);
}

Bool  MySQLRecordset::AsStdString(const char * column, std::string &value)
{
    int columnIndex = FindColumnIndex(column);

    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    m_isNull = *m_ResultRow[columnIndex].is_null;

    if (m_isNull)
    {
        assert(0);
        return FALSE;
    }

    int lenCpy = *m_ResultRow[columnIndex].length;
    value.assign((char*)(m_ResultRow[columnIndex].buffer), lenCpy);
  
    return TRUE;
}

Int MySQLRecordset::AsBLOB(const char * column, Byte * pBuf, Int MaxSize)
{
    return this->GetValue(column, pBuf, MaxSize);
}

int MySQLRecordset::FindColumnIndex(const char * ColumnName)
{
    if (m_Fields == NULL || m_FieldNum == 0)
    {
        assert(0);
        return -1;
    }

    for (int i = 0; i < m_FieldNum; i++)
    {
        if (strcmp(ColumnName, m_Fields[i].name) == 0)
        {
            return i;
        }
    }

    assert(0);
    return -1;
}

Bool MySQLRecordset::GetInt64(int columnIndex, Int64 & x)
{
    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    m_isNull = *m_ResultRow[columnIndex].is_null;

    if (m_isNull)
    {
        //assert(0);
        return FALSE;
    }

    switch (*m_ResultRow[columnIndex].length)
    {
        case 1:
            x = *reinterpret_cast<Int8 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 2:
            x = *reinterpret_cast<Int16 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 4:
            x = *reinterpret_cast<Int32 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 8:
            x = *reinterpret_cast<Int64 *>(m_ResultRow[columnIndex].buffer);
            break;

        default:
            int lenCpy = sizeof(x) > *m_ResultRow[columnIndex].length ? *m_ResultRow[columnIndex].length : sizeof(x);
            memcpy(&x, m_ResultRow[columnIndex].buffer, lenCpy);
    }

    return TRUE;
}

Bool  MySQLRecordset::GetUInt64(int columnIndex, UInt64 & x)
{
    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    m_isNull = *m_ResultRow[columnIndex].is_null;

    if (m_isNull)
    {
        //assert(0);
        return FALSE;
    }

    switch (*m_ResultRow[columnIndex].length)
    {
        case 1:
            x = *reinterpret_cast<Int8 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 2:
            x = *reinterpret_cast<Int16 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 4:
            x = *reinterpret_cast<Int32 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 8:
            x = *reinterpret_cast<Int64 *>(m_ResultRow[columnIndex].buffer);
            break;

        default:
            int lenCpy = sizeof(x) > *m_ResultRow[columnIndex].length ? *m_ResultRow[columnIndex].length : sizeof(x);
            memcpy(&x, m_ResultRow[columnIndex].buffer, lenCpy);
    }

    return TRUE;
}
Bool  MySQLRecordset::GetDouble(int columnIndex, Double & x)
{
    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    m_isNull = *m_ResultRow[columnIndex].is_null;

    if (m_isNull)
    {
        //assert(0);
        return FALSE;
    }

    switch (*m_ResultRow[columnIndex].length)
    {
        case 1:
            x = *reinterpret_cast<Int8 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 2:
            x = *reinterpret_cast<Int16 *>(m_ResultRow[columnIndex].buffer);
            break;

        case 4:
            if (m_ResultRow[columnIndex].buffer_type == MYSQL_TYPE_FLOAT)
            {
                x = *reinterpret_cast<Float *>(m_ResultRow[columnIndex].buffer);
            }
            else
            {
                x = *reinterpret_cast<Int32 *>(m_ResultRow[columnIndex].buffer);
            }

            break;

        case 8:
            if (m_ResultRow[columnIndex].buffer_type == MYSQL_TYPE_DOUBLE)
            {
                x = *reinterpret_cast<Double *>(m_ResultRow[columnIndex].buffer);
            }
            else
            {
                x = (Double) * reinterpret_cast<Int64 *>(m_ResultRow[columnIndex].buffer);
            }

            break;

        default:
            int lenCpy = sizeof(x) > *m_ResultRow[columnIndex].length ? *m_ResultRow[columnIndex].length : sizeof(x);
            memcpy(&x, m_ResultRow[columnIndex].buffer, lenCpy);
    }

    return TRUE;
}

Bool MySQLRecordset::GetValue(const char * column, Int8 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = (Int8)i64;
    return ret;
}

Bool MySQLRecordset::GetValue(const char * column, UInt8 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = (UInt8)ui64;
    return ret;
}

Bool MySQLRecordset::GetValue(const char * column, Int16 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = (Int16)i64;
    return ret;
}

Bool MySQLRecordset::GetValue(const char * column, UInt16 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = (UInt16)ui64;
    return ret;
}

Bool MySQLRecordset::GetValue(const char * column, Int32 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = (Int32)i64;
    return ret;
}

Bool MySQLRecordset::GetValue(const char * column, UInt32 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = (UInt32)ui64;
    return ret;
}

/*    Bool MySQLRecordset::GetValue(const char * column, Long & x )
    {
        int columnIndex = FindColumnIndex(column);
        Int64 i64;
        Bool ret = GetInt64(columnIndex, i64);
        x = (Long)i64;
        return ret;
    }
    Bool MySQLRecordset::GetValue(const char * column, ULong & x )
    {
        int columnIndex = FindColumnIndex(column);
        UInt64 ui64;
        Bool ret = GetUInt64(columnIndex, ui64);
        x = (ULong)ui64;
        return ret;
    }*/
Bool MySQLRecordset::GetValue(const char * column, Int64 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = i64;
    return ret;
}
Bool MySQLRecordset::GetValue(const char * column, UInt64 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = ui64;
    return ret;
}

Bool MySQLRecordset::GetValue(const char * column, Float & x)
{
    int columnIndex = FindColumnIndex(column);
    Double dval;
    Bool ret = GetDouble(columnIndex, dval);
    x = (Float)dval;
    return ret;
}

Bool  MySQLRecordset::GetValue(const char * column, Double & x)
{
    int columnIndex = FindColumnIndex(column);
    Double dval;
    Bool ret = GetDouble(columnIndex, dval);
    x = dval;
    return ret;
}

Bool MySQLRecordset::GetValue(const char * column, DBDateTime & x)
{
    int columnIndex = FindColumnIndex(column);

    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    m_isNull = *m_ResultRow[columnIndex].is_null;

    if (m_isNull)
    {
        //assert(0);
        return FALSE;
    }

    MYSQL_TIME mt;
    int lenCpy = sizeof(mt) > *m_ResultRow[columnIndex].length ? *m_ResultRow[columnIndex].length : sizeof(mt);
    memcpy(&mt, m_ResultRow[columnIndex].buffer, lenCpy);
    x.Year = mt.year;
    x.Month = mt.month;
    x.Day = mt.day;
    x.Hour = mt.hour;
    x.Minute = mt.minute;
    x.Second = mt.second;
    x.Milliseconds = 0;
    x.TimeZone = 0;
    return TRUE;
}

char*  MySQLRecordset::GetValue(const char * column, char* x, Int MaxLength)
{
    int columnIndex = FindColumnIndex(column);

    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return NULL;
    }

    m_isNull = *m_ResultRow[columnIndex].is_null;

    if (m_isNull)
    {
        //assert(0);
        return NULL;
    }

    int lenCpy = (MaxLength - 1) * sizeof(char) > *m_ResultRow[columnIndex].length ? *m_ResultRow[columnIndex].length :
                 (MaxLength - 1) * sizeof(char);
    memcpy(x, m_ResultRow[columnIndex].buffer, lenCpy);
    x[lenCpy] = _T('\0');
    return x;
}

Int MySQLRecordset::GetValue(const char * column, Byte * x, Int MaxSize)
{
    int columnIndex = FindColumnIndex(column);

    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return NULL;
    }

    m_isNull = *m_ResultRow[columnIndex].is_null;

    if (m_isNull)
    {
        //assert(0);
        return NULL;
    }

    int lenCpy = MaxSize > (int) * m_ResultRow[columnIndex].length ? *m_ResultRow[columnIndex].length : MaxSize;
    memcpy(x, m_ResultRow[columnIndex].buffer, lenCpy);
    return lenCpy;
}


//////////////////////////////////////////////////////////////////////////

MySQLParameter::MySQLParameter()
{

}

MySQLParameter::MySQLParameter(Int8 i)
{
    this->i8val = i;
    this->mDataType = ptInt8;
}
MySQLParameter::MySQLParameter(UInt8 i)
{
    this->ui8val = i;
    this->mDataType = ptUInt8;
}
MySQLParameter::MySQLParameter(Int16 i)
{
    this->i16val = i;
    this->mDataType = ptInt16;
}
MySQLParameter::MySQLParameter(UInt16 i)
{
    this->ui16val = i;
    this->mDataType = ptUInt16;
}
MySQLParameter::MySQLParameter(Int32 i)
{
    this->i32val = i;
    this->mDataType = ptInt32;
}
MySQLParameter::MySQLParameter(UInt32 i)
{
    this->ui32val = i;
    this->mDataType = ptUInt32;
}


/*    MySQLParameter::MySQLParameter( Long l ){
        this->lval = l;
        this->mDataType = ptLong;
    }

    MySQLParameter::MySQLParameter( ULong l ){
        this->lval = l;
        this->mDataType = ptULong;
    }*/

MySQLParameter::MySQLParameter(Int64 ll)
{
    this->i64val = ll;
    this->mDataType = ptInt64;
}
MySQLParameter::MySQLParameter(UInt64 ll)
{
    this->ui64val = ll;
    this->mDataType = ptUInt64;
}


MySQLParameter::MySQLParameter(Double d, ParameterType pt)
{
    this->dval = d;
    this->mDataType = pt;
}

MySQLParameter::MySQLParameter(const DBDateTime & dt)
{
    this->mDataType = ptDate;
    this->dtval = dt;
}


MySQLParameter::MySQLParameter(char* str)
{
    this->mDataType = ptString;
    this->strval = str;
}

MySQLParameter::MySQLParameter(const Byte blob[], Int size)
{
    this->mDataType = ptBinary;
    this->pBlob = (Byte *)blob;
    this->mSize = size;
}

void MySQLParameter::setDirection(const Int d)
{

}
MySQLParameter::~MySQLParameter()
{

}

//////////////////////////////////////////////////////////////////////////

MySQLParameterset::MySQLParameterset()
{
    maxcount = 128;
    pparas = (MySQLParameter **) malloc(sizeof(MySQLParameter *) * maxcount);
    memset(pparas, 0, sizeof(MySQLParameter *) * maxcount);
    count = 0;
}


void MySQLParameterset::Append(Int8 i)
{
    this->pparas[count++] = new MySQLParameter(i);
}

void MySQLParameterset::Append(UInt8 i)
{
    this->pparas[count++] = new MySQLParameter(i);
}

void MySQLParameterset::Append(Int16 i)
{
    this->pparas[count++] = new MySQLParameter(i);
}

void MySQLParameterset::Append(UInt16 i)
{
    this->pparas[count++] = new MySQLParameter(i);
}

void MySQLParameterset::Append(Int32 i)
{
    this->pparas[count++] = new MySQLParameter(i);
}

void MySQLParameterset::Append(UInt32 i)
{
    this->pparas[count++] = new MySQLParameter(i);
}

//void MySQLParameterset::Append(Long l){
//    this->pparas[count++] = new MySQLParameter(l);
//}

//void MySQLParameterset::Append(ULong l){
//    this->pparas[count++] = new MySQLParameter(l);
//}

void MySQLParameterset::Append(Double d, ParameterType pt)
{
    this->pparas[count++] = new MySQLParameter(d, pt);
}

void MySQLParameterset::Append(DBDateTime & dt)
{
    this->pparas[count++] = new MySQLParameter(dt);
}

void MySQLParameterset::Append(char* str)
{
    this->pparas[count++] = new MySQLParameter(str);
}

void MySQLParameterset::Append(Byte blob[], Int size)
{
    this->pparas[count++] = new MySQLParameter(blob, size);
}

//     void MySQLParameterset::Append(void * p, ParameterType pt){
//         this->pparas[count++] = new MySQLParameter(p, pt);
//     }

MySQLParameter & MySQLParameterset::Get(Int idx)
{
    return *(this->pparas[idx]);
}


Int  MySQLParameterset::GetSize()
{
    return count;
}


void MySQLParameterset::Clear()
{
    for (Int i  = 0; i < this->count; i ++)
    {
        delete this->pparas[i];
    }

    count = 0;
}


MySQLParameterset::~MySQLParameterset()
{
    this->Clear();
    free(this->pparas);
}

NoSTMTSQLRecordset::NoSTMTSQLRecordset()
{
    m_res = NULL;
    m_num_rows = 0;
    m_row_position = 0;
    m_FieldNum = 0;
    m_row = NULL;
    m_IsNull = NULL;
    m_Err = NULL;
    m_Fields = NULL;
    m_isNull = FALSE;
}

NoSTMTSQLRecordset::~NoSTMTSQLRecordset()
{
    if (m_res != NULL)
    {
        mysql_free_result(m_res);
    }
}

bool NoSTMTSQLRecordset::BindResult(MYSQL * mysql)
{
    //绑定结果
    int nRet = 0;
    m_res = mysql_store_result(mysql);

    if (m_res)
    {
        m_Fields = mysql_fetch_fields(m_res);
        m_FieldNum = mysql_num_fields(m_res);

        if (m_FieldNum > 0 && m_Fields)
        {
            m_num_rows = (int)mysql_affected_rows(mysql);
            m_row_position = 0;
        }
    }
    else
    {
        nRet = 1;
    }

    return nRet == 0;
}
Bool NoSTMTSQLRecordset::IsEmpty(Bool * Err)
{
    if (Err)
    {
        *Err = TRUE;
    }

    return (m_num_rows == 0);
}

Bool NoSTMTSQLRecordset::IsBOF(Bool * Err)
{
    if (Err)
    {
        *Err = TRUE;
    }

    return (m_row_position < 1);
}

Bool NoSTMTSQLRecordset::IsAOL(Bool * Err)
{
    if (Err)
    {
        *Err = TRUE;
    }

    return (m_row_position > m_num_rows);
}

Bool NoSTMTSQLRecordset::MoveFirst()
{
    return FALSE;
}
Bool NoSTMTSQLRecordset::MoveNext()
{
    if (m_row_position + 1 <= m_num_rows)
    {
        m_row_position++;
        m_row = mysql_fetch_row(m_res);

        if (m_row == NULL)
        {
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

Int8 NoSTMTSQLRecordset::AsInt8(const char * column, Bool * Err)
{
    Int8 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Int16 NoSTMTSQLRecordset::AsInt16(const char * column, Bool * Err)
{
    Int16 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Int32 NoSTMTSQLRecordset::AsInt32(const char * column, Bool * Err)
{
    Int32 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

UInt8 NoSTMTSQLRecordset::AsUInt8(const char * column, Bool * Err)
{
    UInt8 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

UInt16 NoSTMTSQLRecordset::AsUInt16(const char * column, Bool * Err)
{
    UInt16 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

UInt32 NoSTMTSQLRecordset::AsUInt32(const char * column, Bool * Err)
{
    UInt32 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
Int64 NoSTMTSQLRecordset::AsInt64(const char * column, Bool * Err)
{
    Int64 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
UInt64 NoSTMTSQLRecordset::AsUInt64(const char * column, Bool * Err)
{
    UInt64 v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Byte NoSTMTSQLRecordset::AsByte(const char * column, Bool * Err)
{
    Byte v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

Int NoSTMTSQLRecordset::AsInteger(const char * column, Bool * Err)
{
    Int v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}

//Long NoSTMTSQLRecordset::AsLong1(const char * column,Bool * Err){
//    Long v;
//    Bool b = this->GetValue(column,v);
//    if(Err) {*Err = b;if(!b) return 0;};
//    return v;
//}

Float NoSTMTSQLRecordset::AsFloat(const char * column, Bool * Err)
{
    Float v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
Double  NoSTMTSQLRecordset::AsDouble(const char * column, Bool * Err)
{
    Double v;
    Bool b = this->GetValue(column, v);

    if (Err)
    {
        *Err = b;

        if (!b)
        {
            return 0;
        }
    };

    return v;
}
Bool NoSTMTSQLRecordset::AsDateTime(const char * column, DBDateTime & dt)
{
    return this->GetValue(column, dt);
}

char* NoSTMTSQLRecordset::AsString(const char * column, char* strBuf, Int MaxLength)
{
    return this->GetValue(column, strBuf, MaxLength);
}

Int NoSTMTSQLRecordset::AsBLOB(const char * column, Byte * pBuf, Int MaxSize)
{
    return this->GetValue(column, pBuf, MaxSize);
}

int NoSTMTSQLRecordset::FindColumnIndex(const char * ColumnName)
{
    if (m_Fields == NULL || m_FieldNum == 0)
    {
        assert(0);
        return -1;
    }

    for (int i = 0; i < m_FieldNum; i++)
    {
        if (strcmp(ColumnName, m_Fields[i].name) == 0)
        {
            return i;
        }
    }

    assert(0);
    return -1;
}

Bool NoSTMTSQLRecordset::GetInt64(int columnIndex, Int64 & x)
{
    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    char * column =  m_row[columnIndex];

    if (column == NULL)
    {
        assert(0);
        return FALSE;
    }

    sscanf(column, "%" PRId64" ", &x);
//      memset(&x,0,sizeof(x));
//      int lenCpy = sizeof(x) > strlen(column) ? strlen(column):sizeof(x);
//      memcpy(&x, column, lenCpy);
    return TRUE;
}

Bool  NoSTMTSQLRecordset::GetUInt64(int columnIndex, UInt64 & x)
{
    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    char * column =  m_row[columnIndex];

    if (column == NULL)
    {
        assert(0);
        return FALSE;
    }

    sscanf(column, "%" PRIu64" ", &x);
    //      memset(&x,0,sizeof(x));
    //      int lenCpy = sizeof(x) > strlen(column) ? strlen(column):sizeof(x);
    //      memcpy(&x, column, lenCpy);
    return TRUE;
}
Bool  NoSTMTSQLRecordset::GetDouble(int columnIndex, Double & x)
{
    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    char * column =  m_row[columnIndex];

    if (column == NULL)
    {
        assert(0);
        return FALSE;
    }

    sscanf(column, "%lf", &x);
    //      memset(&x,0,sizeof(x));
    //      int lenCpy = sizeof(x) > strlen(column) ? strlen(column):sizeof(x);
    //      memcpy(&x, column, lenCpy);
    return TRUE;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, Int8 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = (Int8)i64;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, UInt8 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = (UInt8)ui64;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, Int16 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = (Int16)i64;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, UInt16 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = (UInt16)ui64;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, Int32 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = (Int32)i64;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, UInt32 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = (UInt32)ui64;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, Int64 & x)
{
    int columnIndex = FindColumnIndex(column);
    Int64 i64;
    Bool ret = GetInt64(columnIndex, i64);
    x = i64;
    return ret;
}
Bool NoSTMTSQLRecordset::GetValue(const char * column, UInt64 & x)
{
    int columnIndex = FindColumnIndex(column);
    UInt64 ui64;
    Bool ret = GetUInt64(columnIndex, ui64);
    x = ui64;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, Float & x)
{
    int columnIndex = FindColumnIndex(column);
    Double dval;
    Bool ret = GetDouble(columnIndex, dval);
    x = (Float)dval;
    return ret;
}

Bool  NoSTMTSQLRecordset::GetValue(const char * column, Double & x)
{
    int columnIndex = FindColumnIndex(column);
    Double dval;
    Bool ret = GetDouble(columnIndex, dval);
    x = dval;
    return ret;
}

Bool NoSTMTSQLRecordset::GetValue(const char * column, DBDateTime & x)
{
    int columnIndex = FindColumnIndex(column);

    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return FALSE;
    }

    char * colomn = m_row[columnIndex];

    if (colomn == NULL)
    {
        assert(0);
        return FALSE;
    }

    if (strlen(colomn) != 26)
    {
        return FALSE;
    }

    char tmp[8] = {0};
    memcpy(tmp, colomn, 4);
    x.Year = atoi(tmp);
    memset(tmp, 0, 8);
    memcpy(tmp, colomn + 5, 2);
    x.Month = atoi(tmp);
    memset(tmp, 0, 8);
    memcpy(tmp, colomn + 8, 2);
    x.Day = atoi(tmp);
    memset(tmp, 0, 8);
    memcpy(tmp, colomn + 11, 2);
    x.Hour = atoi(tmp);
    memset(tmp, 0, 8);
    memcpy(tmp, colomn + 14, 2);
    x.Minute = atoi(tmp);
    memset(tmp, 0, 8);
    memcpy(tmp, colomn + 17, 2);
    x.Second = atoi(tmp);
    memset(tmp, 0, 8);
    memcpy(tmp, colomn + 20, 6);
    x.Milliseconds = atoi(tmp);
    //strptime("2010-11-15 10:39:30.123456", "%Y-%m-%d %H:%M:%S", &tm_time);
    //sscanf(colomn,"%04d-%02d-%02d %02d:%02d:%02d.%d",x.Year,x.Month,x.Day,x.Hour,x.Minute,x.Second,x.Milliseconds);
    return TRUE;
}

char*  NoSTMTSQLRecordset::GetValue(const char * column, char* x, Int MaxLength)
{
    int columnIndex = FindColumnIndex(column);

    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return NULL;
    }

    char * colomn = m_row[columnIndex];

    if (colomn == NULL)
    {
        assert(0);
        return NULL;
    }

    int lenCpy = (MaxLength - 1) * sizeof(char) > strlen(colomn) ? strlen(colomn) : (MaxLength - 1) * sizeof(char);
    memcpy(x, colomn, lenCpy);
    x[lenCpy] = _T('\0');
    return x;
}

Int NoSTMTSQLRecordset::GetValue(const char * column, Byte * x, Int MaxSize)
{
    int columnIndex = FindColumnIndex(column);

    if (columnIndex < 0 || columnIndex >= m_FieldNum)
    {
        m_isNull = TRUE;
        assert(0);
        return NULL;
    }

    char * colomn = m_row[columnIndex];

    if (colomn == NULL)
    {
        assert(0);
        return NULL;
    }

    int lenCpy = MaxSize > (int) strlen(colomn) ? strlen(colomn) : MaxSize;
    memcpy(x, colomn, lenCpy);
    return lenCpy;
}

}
}

