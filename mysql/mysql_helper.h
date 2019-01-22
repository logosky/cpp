#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#ifndef __MYSQL_HELPER_H__
#define __MYSQL_HELPER_H__


#ifdef WIN32
#include <tchar.h>
#endif


#include "mysql_comm.h"
#include <mysql/mysql.h>



namespace demo
{
namespace mysql
{

struct st_buffer_size_type
{
    char * buffer;
    size_t size;
    enum_field_types type;
    st_buffer_size_type(char * b, size_t s, enum_field_types t) : buffer(b), size(s), type(t) {}
};
st_buffer_size_type allocate_buffer_for_field(const MYSQL_FIELD * const field);



int DBTimeToString(DBDateTime & dt, char* str, int strMaxLen);

Bool StringToDBTime(DBDateTime & dt, char* str);

}
}


#endif // !defined(_DB_COMMON_HELPER_H)
