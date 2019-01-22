#ifndef __MYSQL_CONNECT_H__
#define __MYSQL_CONNECT_H__

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#endif

#include "mysql_comm.h"
#include "mysql_interface.h"
#include <boost/detail/atomic_count.hpp>
#include <mysql/mysql.h>


namespace demo
{
namespace mysql
{

class MySQLConnection;
class MySQLCommand;
class MySQLRecordset;
class MySQLParameter;
class MySQLParameterset;



class  MySQLConnection:public IConnection
{
public:
    MySQLConnection();
    virtual ~MySQLConnection();

    virtual Bool SetAccessUrl(const char* strPath);
    virtual Bool SetPass(const char* strUser, const char* strPwd);
    virtual Bool SetPassword(const char* strPwd);

    void SetReadTimeout(unsigned int read_timeout);

    virtual Bool Open();
    virtual Bool Close();

    //返回0代表成功，非0代表失败
    virtual long BeginTrans();
    virtual Bool RollbackTrans();
    virtual Bool CommitTrans();

    /**
     * @brief: 转义需要插入或者修改的字符串，转义时会按照当前连接的字符集，而不是直接按照
     *          纯的ASCII字符串进行转义
     *         必须保证ouput的长度大于等于 input_len * 2 + 1
     *
     *
     * @param input[in] 需要转义的字符串
     * @param input_len[in] 需要转义的字符串长度
     * @param output[out] 转义完成后的字符串
     *
     * @return 返回ouput
     */
    char* EscapeString(const char * input, size_t input_len, char * output);

    virtual ICommand * CreateCommand(const char* CmdTxt  = NULL);

    virtual UInt64 LastInsertID(Bool * Err = NULL);

    //错误码
    virtual UInt32 GetLastError();
    //错误信息
    virtual char* GetLastErrorMessage(char* strBuf, Int MaxSize);
    //SHOW ERRORS返回的错误
    virtual char* GetSQLError(char* strBuf, Int MaxSize);

    /**
     * @brief: 上一次执行错误信息
     *
     * @return 返回ouput
     */
    virtual const char* GetLastErrorMessage();

    // SHOW ERRORS返回的错误
    virtual const char* GetSQLError();

    static void MySQL_Init();

    static void MySQL_End();

private:
    char mstrUsername[256];
    char mstrPassword[256];
    char mstrAccessUrl[1024];
    char m_last_sys_error[1024];
    char m_last_sql_error[1024];
    short m_Port;
    MYSQL m_conn;
    friend class MySQLCommand;

    unsigned int m_ReadTimeout;
};

class  MySQLCommand:
    public ICommand
{
public:
    static boost::detail::atomic_count exec_cmd_count;
public:
    MySQLCommand(MySQLConnection & conn);
    ~MySQLCommand();
    /*
    初始化动态参数绑定的命令
    */
    virtual Bool PrepareCommand(const char* CmdTxt);
    /*
    执行更新操作，不产生结果集，返回修改的行数。
    函数返回0值可能是未找到更新条件匹配的行，也可能是发生了错误，需要检查GetLastError确定是何种情况。
    */
    virtual Int ExecuteNoQuery();
    /*
    直接执行入参的sql语句查询操作，返回一个结果集，返回结果集代表语句执行成功，否则返回NULL
    注意：如果返回的结果集中未包含任何行，说明查询语句执行成功，但未检索到匹配行
    */
    virtual IRecordset * ExecuteQueryNoSTMT(const char* CmdTxt, IRecordset & rs);
    /*
    执行查询操作，返回一个结果集,返回结果集代表语句执行成功，否则返回NULL
    注意：如果返回的结果集中未包含任何行，说明查询语句执行成功，但未检索到匹配行
    */
    virtual IRecordset * ExecuteQuery(IRecordset & rs);

    //直接执行命令，不使用参数绑定，参数绑定执行方式有些限制，此函数无法获得结果集
    virtual Int ExecuteNoQuery(const char* CmdTxt);
    //MySQLRecordset * ExecuteQuery( char* CmdTxt, MySQLRecordset& rs );

    virtual Bool AppendParameter(const Int8 i);
    virtual Bool AppendParameter(const UInt8 i);
    virtual Bool AppendParameter(const Int16 i);
    virtual Bool AppendParameter(const UInt16 i);
    virtual Bool AppendParameter(const Int32 i);
    virtual Bool AppendParameter(const UInt32 i);
    //virtual Bool AppendParameter(const Long l);
    //virtual Bool AppendParameter(const ULong l);
    virtual Bool AppendParameter(const Int64 ll);
    virtual Bool AppendParameter(const UInt64 ll);
    virtual Bool AppendParameter(const Double d, ParameterType pt = ptDouble);
    virtual Bool AppendParameter(const DBDateTime & dt);
    virtual Bool AppendParameter(char* str);
    virtual Bool AppendParameter(const Byte blob[], Int size);
    //virtual Bool AppendParameter(const void * p, ParameterType pt);
    virtual Bool AppendParameter(const IParameter & prm);
    virtual Bool AppendParameter(const IParameterset & ps);

    virtual Bool AppendParameter(MySQLParameter & prm);
    virtual Bool AppendParameter(MySQLParameterset & ps);

protected:
    int     m_nParamterAppended;
    int     m_nParamCount;
    MYSQL_BIND * m_Params;
private:
    MySQLConnection * m_Connection;
    MYSQL_STMT   *  m_Statement;
    char*          m_Cmd;
    friend class MySQLRecordset;
    friend class MySQLConnection;
};

class  MySQLRecordset:
    public IRecordset
{
public:
    MySQLRecordset();
    ~MySQLRecordset();

    Bool IsEmpty(Bool * Err = NULL);
    Bool IsAOL(Bool * Err = NULL);//is after of last
    Bool IsBOF(Bool * Err = NULL);//is before of first

    Bool MoveFirst();
    Bool MoveNext();

    //返回上一次获取的字段值是否是空值
    //在GetValue或者AsXXX函数调用之后调用，可用来判断先前获得的值是否有效
    Bool IsNull();

    Int8    AsInt8(const char * column, Bool * Err = NULL);
    UInt8   AsUInt8(const char * column, Bool * Err = NULL);
    Int16   AsInt16(const char * column, Bool * Err = NULL);
    UInt16  AsUInt16(const char * column, Bool * Err = NULL);
    Int32   AsInt32(const char * column, Bool * Err = NULL);
    UInt32  AsUInt32(const char * column, Bool * Err = NULL);
    Int64   AsInt64(const char * column, Bool * Err = NULL);
    UInt64  AsUInt64(const char * column, Bool * Err = NULL);

    Byte    AsByte(const char * column, Bool * Err = NULL);
    Int     AsInteger(const char * column, Bool * Err = NULL);
    //Long    AsLong1(const char * column,Bool * Err = NULL);
    Float   AsFloat(const char * column, Bool * Err = NULL);
    Double  AsDouble(const char * column, Bool * Err = NULL);
    Bool    AsDateTime(const char * column, DBDateTime & dt);


    char*  AsString(const char * column, char* strBuf, Int MaxLength = 1024);

    Bool  AsStdString(const char * column, std::string &value);
    Int     AsBLOB(const char * column, Byte * pBuf, Int MaxSize);

    //返回TRUE代表值非空，FALSE代表结果集中对应列为空或未找到该列
    Bool    GetValue(const char * column, Int8 & x);
    Bool    GetValue(const char * column, UInt8 & x);
    Bool    GetValue(const char * column, Int16 & x);
    Bool    GetValue(const char * column, UInt16 & x);
    Bool    GetValue(const char * column, Int32 & x);
    Bool    GetValue(const char * column, UInt32 & x);
    //Bool    GetValue(const char * column, Long & x );
    //Bool  GetValue(const char * column, ULong & x );
    Bool    GetValue(const char * column, Int64 & x);
    Bool    GetValue(const char * column, UInt64 & x);
    Bool    GetValue(const char * column, Float & x);
    Bool    GetValue(const char * column, Double & x);
    Bool    GetValue(const char * column, DBDateTime & x);
    char*  GetValue(const char * column, char*  x, Int MaxLength = 1024);
    Int     GetValue(const char * column, Byte  * x, Int MaxSize = 1024);


protected:
    bool BindResult(MySQLCommand * command);
    int  FindColumnIndex(const char * ColumnName);
    Bool  GetInt64(const int ColumnIndex, Int64 & x);
    Bool  GetUInt64(const int ColumnIndex, UInt64 & x);
    Bool  GetDouble(const int ColumnIndex, Double & x);
protected:
    MYSQL_RES   *   m_ResultSetMetadata;
    MySQLCommand  * m_Command;
    int             m_num_rows;
    int             m_row_position;
    MYSQL_BIND   *  m_ResultRow;
    unsigned long * m_DataLen;
    char      *     m_IsNull;
    char      *     m_Err;
    int             m_FieldNum;
    MYSQL_FIELD  *  m_Fields;
    friend class MySQLCommand;
    Bool            m_isNull;
};
class  NoSTMTSQLRecordset: public IRecordset
{
public:
    NoSTMTSQLRecordset();
    ~NoSTMTSQLRecordset();

    Bool IsEmpty(Bool * Err = NULL);
    Bool IsAOL(Bool * Err = NULL);//is after of last
    Bool IsBOF(Bool * Err = NULL);//is before of first

    Bool MoveFirst();
    Bool MoveNext();

    //返回上一次获取的字段值是否是空值
    //在GetValue或者AsXXX函数调用之后调用，可用来判断先前获得的值是否有效
    Bool IsNull();

    Int8    AsInt8(const char * column, Bool * Err = NULL);
    UInt8   AsUInt8(const char * column, Bool * Err = NULL);
    Int16   AsInt16(const char * column, Bool * Err = NULL);
    UInt16  AsUInt16(const char * column, Bool * Err = NULL);
    Int32   AsInt32(const char * column, Bool * Err = NULL);
    UInt32  AsUInt32(const char * column, Bool * Err = NULL);
    Int64  AsInt64(const char * column, Bool * Err = NULL);
    UInt64 AsUInt64(const char * column, Bool * Err = NULL);

    Byte    AsByte(const char * column, Bool * Err = NULL);
    Int     AsInteger(const char * column, Bool * Err = NULL);
    //Long    AsLong1(const char * column,Bool * Err = NULL);
    Float   AsFloat(const char * column, Bool * Err = NULL);
    Double  AsDouble(const char * column, Bool * Err = NULL);
    Bool    AsDateTime(const char * column, DBDateTime & dt);


    char*  AsString(const char * column, char* strBuf, Int MaxLength = 1024);
    Int     AsBLOB(const char * column, Byte * pBuf, Int MaxSize);

    //返回TRUE代表值非空，FALSE代表结果集中对应列为空或未找到该列
    Bool    GetValue(const char * column, Int8 & x);
    Bool    GetValue(const char * column, UInt8 & x);
    Bool    GetValue(const char * column, Int16 & x);
    Bool    GetValue(const char * column, UInt16 & x);
    Bool    GetValue(const char * column, Int32 & x);
    Bool    GetValue(const char * column, UInt32 & x);
    //Bool    GetValue(const char * column, Long & x );
    //Bool GetValue(const char * column, ULong & x );
    Bool    GetValue(const char * column, Int64 & x);
    Bool   GetValue(const char * column, UInt64 & x);
    Bool    GetValue(const char * column, Float & x);
    Bool    GetValue(const char * column, Double & x);
    Bool    GetValue(const char * column, DBDateTime & x);
    char*  GetValue(const char * column, char*  x, Int MaxLength = 1024);
    Int     GetValue(const char * column, Byte  * x, Int MaxSize = 1024);


protected:
    bool BindResult(MYSQL * mysql);
    int  FindColumnIndex(const char * ColumnName);
    Bool  GetInt64(const int ColumnIndex, Int64 & x);
    Bool  GetUInt64(const int ColumnIndex, UInt64 & x);
    Bool  GetDouble(const int ColumnIndex, Double & x);
protected:
    MYSQL_RES   *  m_res;
    int            m_num_rows;
    int            m_row_position;
    int            m_FieldNum;
    MYSQL_ROW      m_row;
    char     *     m_IsNull;
    char     *     m_Err;
    MYSQL_FIELD  * m_Fields;
    friend class MySQLCommand;
    Bool           m_isNull;
};

class  MySQLParameter:
    public IParameter
{
public:
    MySQLParameter();

    MySQLParameter(const Int8 i);//MySQLParameter(Byte b);
    MySQLParameter(const UInt8 i);
    MySQLParameter(const Int16 i);
    MySQLParameter(const UInt16 i);
    MySQLParameter(const Int32 i);//MySQLParameter(Int i);
    MySQLParameter(const UInt32 i);//MySQLParameter(DBKey i);

    //MySQLParameter(const Long l);//MySQLParameter(long l);
    //MySQLParameter(const ULong l);

    MySQLParameter(const Int64 ll);
    MySQLParameter(const UInt64 ll);

    MySQLParameter(const Double d, const ParameterType pt = ptDouble);

    MySQLParameter(const DBDateTime & dt);
    MySQLParameter(char* str);
    MySQLParameter(const Byte blob[], const Int size);

    void setDirection(const Int d);
    ~MySQLParameter();
protected:
    ParameterType mDataType;

    Int8   i8val;
    UInt8  ui8val;//UInt8
    Int16  i16val;
    UInt16 ui16val;
    Int32    i32val;
    UInt32   ui32val;
    Long   lval;
    ULong   ulval;
    Int64   i64val;
    UInt64  ui64val;

    Double dval;
    DBDateTime dtval;
    char* strval;
    Byte * pBlob;
    void * pv;


    Int mParameterDirection;
    Int mSize;
    friend class MySQLCommand;
private:

};


/**
*@brief     用于存放参数集
*
*         用于存放参数集.
*/
class  MySQLParameterset:
    public IParameterset
{
public:
    MySQLParameterset();

    void Append(MySQLParameter & pare);


    void Append(Int8 i);//void Append(Byte b);
    void Append(UInt8 i);
    void Append(Int16 i);
    void Append(UInt16 i);
    void Append(Int32 i);//void Append(Int i);
    void Append(UInt32 i);//void Append(DBKey i);

    void Append(Long l);//void Append(long l);
    void Append(ULong l);

    void Append(Double d, ParameterType pt = ptDouble);


    void Append(DBDateTime & dt);
    void Append(char* str);
    void Append(Byte blob[], Int size);
    // void Append(void * p, ParameterType pt);

    MySQLParameter & Get(Int idx);

    Int  GetSize();
    void Clear();


    ~MySQLParameterset();
private:
    MySQLParameter ** pparas;
    Int maxcount;
    Int count;
};

}
}

#endif
