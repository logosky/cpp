#include "db_pch.h"


#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mysql_helper.h"

namespace demo
{
namespace mysql
{

st_buffer_size_type allocate_buffer_for_field(const MYSQL_FIELD * const field)
{
    switch (field->type)
    {
        case MYSQL_TYPE_NULL:
            return st_buffer_size_type(NULL, 0, field->type);

        case MYSQL_TYPE_TINY:
            return st_buffer_size_type(new char[1], 1, field->type);

        case MYSQL_TYPE_SHORT:
            return st_buffer_size_type(new char[2], 2, field->type);

        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_FLOAT:
            return st_buffer_size_type(new char[4], 4, field->type);

        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_LONGLONG:
            return st_buffer_size_type(new char[8], 8, field->type);

        case MYSQL_TYPE_YEAR:
            return st_buffer_size_type(new char[2], 2, MYSQL_TYPE_SHORT);

        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
            return st_buffer_size_type(new char[sizeof(MYSQL_TIME)], sizeof(MYSQL_TIME), field->type);


        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
            return st_buffer_size_type(new char[field->length + 1], field->length + 1, field->type);

        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            return st_buffer_size_type(new char[64], 64, field->type);
#if A1

        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_YEAR:
            return st_buffer_size_type(new char[10], 10, field->type);
#endif
#if A0

            // There two are not sent over the wire
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET:
#endif
        case MYSQL_TYPE_BIT:
            return st_buffer_size_type(new char[8], 8, MYSQL_TYPE_BIT);

        case MYSQL_TYPE_GEOMETRY:
        default:
            ;
    }

    return st_buffer_size_type(NULL, 0, field->type);
}


int DBTimeToString(DBDateTime & dt, char* str, int strMaxLen)
{
    if (strMaxLen < 20)
    {
        return 0;
    }

    sprintf(str, "%d-%d-%d %d:%d:%d", dt.Year, dt.Month, dt.Day, dt.Hour, dt.Minute, dt.Second);
    return strlen(str);
}

Bool StringToDBTime(DBDateTime & dt, char* str)
{
    memset(&dt, 0, sizeof(dt));
    //分割字符串
    char  val[6][12];
    memset(val, 0, 6 * 12 * sizeof(char));
    int valCount = 0;
    char * pos = str;
    char * tpos = str;

    //删除前导空格
    while (*pos == _T(' ') && *pos != _T('\0'))
    {
        pos++;
    }

    tpos = pos;

    while (*pos != _T('\0'))
    {
        if (*pos == _T('-') || *pos == _T(':') || *pos == _T(' ') || *pos == _T('\0'))
        {
            memcpy(val[valCount], tpos, (pos - tpos)*sizeof(char));
            valCount++;

            if (valCount >= 6)
            {
                break;
            }

            pos++;

            //删除多余空格
            while (*pos == _T(' ') && *pos != _T('\0'))
            {
                pos++;
            }

            tpos = pos;
        }
        else
        {
            pos++;
        }
    }

    if (valCount < 6)
    {
        memcpy(val[valCount], tpos, (pos - tpos)*sizeof(char));
        valCount++;
    }

    if (valCount >= 1)
    {
        dt.Year = _tstoi(val[0]);
    }

    if (valCount >= 2)
    {
        dt.Month = _tstoi(val[1]);
    }

    if (valCount >= 3)
    {
        dt.Day = _tstoi(val[2]);
    }

    if (valCount >= 4)
    {
        dt.Hour = _tstoi(val[3]);
    }

    if (valCount >= 5)
    {
        dt.Minute = _tstoi(val[4]);
    }

    if (valCount >= 6)
    {
        dt.Second = _tstoi(val[5]);
    }

    return TRUE;
}

}
}
