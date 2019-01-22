#include "db_pch.h"
#include "mysql_interface.h"

namespace demo
{
namespace mysql
{

IConnection::~IConnection()
{
};

// 添加以下虚函数之实现，避免编译警告
ICommand::~ICommand()
{

}

IRecordset::~IRecordset()
{

}

}
}
