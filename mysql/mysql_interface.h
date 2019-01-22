#ifndef __MYSQL_INTERFACE_H__
#define __MYSQL_INTERFACE_H__

#include "mysql_comm.h"
#include <boost/function.hpp>

#pragma once



namespace demo
{
namespace mysql
{

enum ParameterType
{
    ptByte,
    ptTinyInt,
    ptInt8,
    ptUInt8,
    ptInt16,
    ptUInt16,
    ptInt,
    ptUInt,
    ptInt32,
    ptUInt32,
    ptInt64,
    ptUInt64,
    ptDBKey,
    ptLong,
    ptULong,
    ptFloat,
    ptDouble,
    ptVarChar,
    ptString,
    ptDate,
    ptBlob,
    ptBinary
};

class ICommand;
class IRecordset;
class IParameter;
class IParameterset;


class IConnection
{
public:
    typedef boost::function<bool()> OnCloseCallback;

    virtual ~IConnection();
    virtual Bool SetAccessUrl(const char* strPath) = 0;
    virtual Bool SetPass(const char* strUser, const char* strPwd) = 0;
    virtual Bool SetPassword(const char* strPwd) = 0;

    virtual Bool Open() = 0;
    virtual Bool Close() = 0;

    //返回0代表成功，非0代表失败
    virtual long BeginTrans() = 0;
    virtual Bool RollbackTrans() = 0;
    virtual Bool CommitTrans() = 0;

    virtual ICommand * CreateCommand(const char* CmdTxt = NULL) = 0;

    virtual UInt64 LastInsertID(Bool * Err = NULL) = 0;

    //错误码
    virtual UInt32 GetLastError() = 0;
    //错误信息
    virtual char* GetLastErrorMessage(char* strBuf, Int MaxSize) = 0;

    //SHOW ERRORS返回的错误
    virtual char* GetSQLError(char* strBuf, Int MaxSize) = 0;
    
    virtual const char* GetLastErrorMessage() = 0;
    virtual const char* GetSQLError() = 0;
};


class ICommand
{
public:
    virtual ~ICommand();

    /*
    初始化动态参数绑定的命令
    */
    virtual Bool PrepareCommand(const char* CmdTxt) = 0;

    /*
    执行更新操作，不产生结果集，返回修改的行数。
    函数返回0值可能是未找到更新条件匹配的行，也可能是发生了错误，需要检查GetLastError确定是何种情况。
    */
    virtual Int ExecuteNoQuery() = 0;
    /*
    直接执行入参的sql语句查询操作，返回一个结果集，返回结果集代表语句执行成功，否则返回NULL
    注意：如果返回的结果集中未包含任何行，说明查询语句执行成功，但未检索到匹配行
    */
    virtual IRecordset * ExecuteQueryNoSTMT(const char* CmdTxt, IRecordset & rs) = 0;
    /*
    执行查询操作，返回一个结果集,返回结果集代表语句执行成功，否则返回NULL
    注意：如果返回的结果集中未包含任何行，说明查询语句执行成功，但未检索到匹配行
    */
    virtual IRecordset * ExecuteQuery(IRecordset & rs) = 0;


    //直接执行命令，不使用参数绑定，参数绑定执行方式有些限制，此函数无法获得结果集
    virtual Int ExecuteNoQuery(const char* CmdTxt) = 0;


    virtual Bool AppendParameter(const Int8 i) = 0;
    virtual Bool AppendParameter(const UInt8 i) = 0;
    virtual Bool AppendParameter(const Int16 i) = 0;
    virtual Bool AppendParameter(const UInt16 i) = 0;
    virtual Bool AppendParameter(const Int32 i) = 0;
    virtual Bool AppendParameter(const UInt32 i) = 0;
    //virtual Bool AppendParameter(const Long l)=0;
    //virtual Bool AppendParameter(const ULong l)=0;
    virtual Bool AppendParameter(const Int64 ll) = 0;
    virtual Bool AppendParameter(const UInt64 ll) = 0;
    virtual Bool AppendParameter(const Double d, ParameterType pt = ptDouble) = 0;
    virtual Bool AppendParameter(const DBDateTime & dt) = 0;
    virtual Bool AppendParameter(char* str) = 0;
    virtual Bool AppendParameter(const Byte blob[], Int size) = 0;
    //virtual Bool AppendParameter(const void * p, ParameterType pt)=0;
    virtual Bool AppendParameter(const IParameter & prm) = 0;
    virtual Bool AppendParameter(const IParameterset & ps) = 0;
};

class IRecordset
{
public:
    virtual ~IRecordset();

    virtual Bool IsEmpty(Bool * Err = NULL) = 0;
    virtual Bool IsAOL(Bool * Err = NULL) = 0; //is after of last
    virtual Bool IsBOF(Bool * Err = NULL) = 0; //is before of first

    virtual Bool MoveFirst() = 0;
    virtual Bool MoveNext() = 0;

    //返回上一次获取的字段值是否是空值
    //在GetValue或者AsXXX函数调用之后调用，可用来判断先前获得的值是否有效
    //virtual Bool IsNull()=0;

    virtual Int8    AsInt8(const char * column, Bool * Err = NULL) = 0;
    virtual UInt8   AsUInt8(const char * column, Bool * Err = NULL) = 0;
    virtual Int16   AsInt16(const char * column, Bool * Err = NULL) = 0;
    virtual UInt16  AsUInt16(const char * column, Bool * Err = NULL) = 0;
    virtual Int32   AsInt32(const char * column, Bool * Err = NULL) = 0;
    virtual UInt32  AsUInt32(const char * column, Bool * Err = NULL) = 0;
    virtual Int64   AsInt64(const char * column, Bool * Err = NULL) = 0;
    virtual UInt64  AsUInt64(const char * column, Bool * Err = NULL) = 0;

    virtual Byte    AsByte(const char * column, Bool * Err = NULL) = 0;
    virtual Int     AsInteger(const char * column, Bool * Err = NULL) = 0;
    //virtual Long    AsLong1(const char * column,Bool * Err = NULL)=0;
    virtual Float   AsFloat(const char * column, Bool * Err = NULL) = 0;
    virtual Double  AsDouble(const char * column, Bool * Err = NULL) = 0;
    virtual Bool    AsDateTime(const char * column, DBDateTime & dt) = 0;


    virtual char*  AsString(const char * column, char* strBuf, Int MaxLength = 1024) = 0;
    virtual Int     AsBLOB(const char * column, Byte * pBuf, Int MaxSize) = 0;

    //返回TRUE代表值非空，FALSE代表结果集中对应列为空或未找到该列
    virtual Bool    GetValue(const char * column, Int8 & x) = 0;
    virtual Bool    GetValue(const char * column, UInt8 & x) = 0;
    virtual Bool    GetValue(const char * column, Int16 & x) = 0;
    virtual Bool    GetValue(const char * column, UInt16 & x) = 0;
    virtual Bool    GetValue(const char * column, Int32 & x) = 0;
    virtual Bool    GetValue(const char * column, UInt32 & x) = 0;
    //virtual Bool    GetValue(const char * column, Long & x )=0;
    //virtual Bool  GetValue(const char * column, ULong & x )=0;
    virtual Bool    GetValue(const char * column, Int64 & x) = 0;
    virtual Bool    GetValue(const char * column, UInt64 & x) = 0;
    virtual Bool    GetValue(const char * column, Float & x) = 0;
    virtual Bool    GetValue(const char * column, Double & x) = 0;
    virtual Bool    GetValue(const char * column, DBDateTime & x) = 0;
    virtual char*  GetValue(const char * column, char*  x, Int MaxLength = 1024) = 0;
    virtual Int     GetValue(const char * column, Byte  * x, Int MaxSize = 1024) = 0;
};


class IParameter
{
public:

protected:
private:
};

class IParameterset
{
public:
protected:
private:
};


}//namespace
}




#endif
