#ifndef MYBASE_H_20140806
#define MYBASE_H_20140806

#include <stdio.h>
#include <iconv.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <vector>
#include <set>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <map>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

using namespace std;

#ifndef uint
typedef unsigned int uint;
#endif

namespace BASE {

typedef struct st_time {
	uint uiYear;
	uint uiMonth;
	uint uiDay;
	uint uiHour;
	uint uiMinute;
	uint uiSecond;
	uint uiDayOfWeek;	// 周几

	string strDate;		// 日期：2010-04-27
	string strTime;		// 时间：12:20:30
	string strDateTime;	// 日期时间：2010-04-27 12:20:30

	uint uiAllTicks;	// 从1970到现在的滴答数

	st_time() :
			uiYear(0), uiMonth(0), uiDay(0), uiHour(0), uiMinute(0), uiSecond(
					0), uiDayOfWeek(0), uiAllTicks(0) {
		strDate = "";
		strTime = "";
		strDateTime = "";
	}
} StTime;

/**
 * 字符串替换
 */
int inline ReplaceString(string &strSource, const string &strKey,
		const string &strValue);

/**
 * 将字符串按分隔符拆分成列表
 * bCleanEmpty为true表示去除空格
 */
int inline SplitToArray(const string &strSource, const string &strDelimiter,
		vector<string> &vec, bool bCleanEmpty = true);

/**
 * 将字符串中的大写字母转化为小写
 */
string inline ToLower(string strSrc);

/**
 * 判断是否是字母或数字
 */
bool inline IsNumbric(char ch);

int inline atoi(string a);
string inline itoa(int i);
float inline atof(string a);
string inline ftoa(float f);
uint inline atou(string a);
string inline utoa(unsigned int u);
string inline tostr(const char* str);

/**
 * 格式化字符串，将字符c进行格式化：如"将被格式化为\"
 */
string inline FormatString(char *pszStr, char c);

/**
 * 从字符串中获取数字
 */
int inline GetNumbricFromString(string &sSrc, vector<uint> &vctRes);

/**************************  时间函数开始 *************************/

/**
 * 将2010-12-1格式的字符串转换成中文格式，如2010年12月1日
 */
string inline GetChineseDate(const string &strDate);

/**
 * 获取某天的日期，日期的格式为：2010-09-02。
 * @[out]pszDate: 保存获取到的日期字符串
 * @[in]iLen: 保存日期字符串的空间长度
 * @[in]iWhichDay: 获取第几天，0为当天，1为明天，2为第三天，以此类推.默认为当天
 * @[in]cSplit: 分隔符，形如'-'或者'/'等.默认为'-'
 * @return:
 - 0   成功
 - -1  失败
 **/
int inline GetWhichDateStr(char *pszDate, int iLen, int iWhichDay = 0,
		char cSplit = '-');
string inline GetWhichDateStr(int iWhichDay = 0, char cSplit = '-');

/**
 * 获取当前时间的字符串,获取得到的时间字符串格式为："2011-04-12 20:30:20"
 */
void inline GetNowTimeStr(char *pszTime, int iLen);
string inline GetNowTimeStr();

/**
 * 获取当前时间
 * 返回时间结构：年，月，日，时，分，秒，日期，时间，日期时间
 */
StTime inline GetNowTime();

/**
 * 根据时间戳获取时间字符串
 */
string inline GetTimeStr(uint uiTime);

/**
 * 获取从当前时间开始，N天后的时间
 * iDay为0表示当前天；为-1表示昨天；为1表示明天。以此类推
 */
StTime inline GetWhichDay(int iDay, int iHour = 0, int iMinute = 0);

/**
 * 根据滴答数获取时间结构
 */
void inline FormatStTime(uint uiTicks, StTime &stTime);

/**
 * 获取从某个时间点开始，N天后的时间
 * iDay为0表示当前天；为-1表示昨天；为1表示明天。以此类推
 */
StTime inline GetWhichDayBaseWhichDay(uint uiTicks, int iDays);
StTime inline GetWhichDayBaseWhichDay(const char *szWhichDay, int iDays);

/**
 * 将时间字符串如"2011-04-12 20:30:20"转化为时间滴答数
 */
time_t inline ConvertToTime(const char *szTime);

/**
 * 将日期时间字符串如"2011-04-12"转化为时间滴答数
 */
time_t inline DateToTime(const char *szTime);

/**
 * 将滴答数转化为时间tm结构：可以支持到1970年 ～ 2106年
 *
 * 功能等效于时间转换函数：localtime
 * 区别在于此函数可以表示的时间范围比较大，localtime的参数是int，而my_localtime_r是unsigned int
 */
int inline my_localtime_r(unsigned int timep, struct tm *result);

/**************************  时间函数结束 *************************/

/**
 * 将字符转为16进制
 */
string inline Char2Hex(char dec);

/**
 * 将16进制转化为字符
 */
char inline Hex2Char(char first, char second);

/**
 * 对url进行编码，将中文和一些特殊字符转为16进制的形式
 */
string inline EncodeURIComponent(const string &c);

/**
 * 对字符串进行解码，与EncodeURIComponent对应
 */
string inline DecodeURIComponent(const string& src);

#ifndef MAX_TIME_DIFF
#define MAX_TIME_DIFF 24*60*60
#endif

#ifndef SECS_PER_HOUR
#define SECS_PER_HOUR   (60 * 60)
#endif

#ifndef SECS_PER_DAY
#define SECS_PER_DAY    (SECS_PER_HOUR * 24)
#endif

const unsigned short int __mon_yday[2][13] = {
/* Normal years.  */
{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
/* Leap years.  */
{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 } };

int inline ReplaceString(string &strSource, const string &strKey,
		const string &strValue) {
	size_t pos = 0;
	size_t n;
	size_t keyLength = strKey.size();
	size_t valueLength = strValue.size();

	while ((n = strSource.find(strKey, pos)) != string::npos) {
		strSource.replace(n, keyLength, strValue);
		pos = n + valueLength;
	}

	return 0;
}

int inline SplitToArray(const string &strSource, const string &strDelimiter,
		vector<string> &vec, bool bCleanEmpty) {
	size_t pos = 0;
	size_t n;

	if (strSource.empty())
		return 0;

	string str = strSource;
	str.append(strDelimiter);
	string substr;

	while ((n = str.find(strDelimiter, pos)) != string::npos) {
		substr = str.substr(pos, n - pos);
		if (!bCleanEmpty || !substr.empty()) {
			vec.push_back(str.substr(pos, n - pos));
		}
		pos = n + strDelimiter.size();
	}

	return 0;
}

int inline atoi(string a) {
	return atoi(a.c_str());
}

string inline itoa(int i) {
	char buf[64];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", i);
	return buf;
}

float inline atof(string a) {
	return atof(a.c_str());
}

string inline ftoa(float f) {
	char buf[64];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%f", f);
	return buf;
}

unsigned int atou(string a) {
	return atoll(a.c_str());
}

string inline utoa(unsigned int u) {
	char buf[64];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%u", u);
	return buf;
}

string inline tostr(const char* str) {
	return str ? str : "";
}

string inline ToLower(string strSrc) {
	for (uint i = 0; i < strSrc.length(); i++) {
		if (strSrc[i] >= 'A' && strSrc[i] <= 'Z')
			strSrc[i] = strSrc[i] + 32;
	}
	return strSrc;
}

bool inline IsNumbric(char ch) {
	if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z')
			|| (ch >= 'A' && ch <= 'Z')) {
		return true;
	}
	return false;
}

int inline GetNumbricFromString(string &sSrc, vector<uint> &vctRes) {
	if (sSrc.length() < 1) {
		vctRes.clear();
		return 0;
	}
	char szTmp[11] = { 0 };
	int j = 0;
	for (uint i = 0; i < sSrc.length(); i++) {
		char &ch = sSrc[i];
		if (ch >= '0' && ch <= '9') {
			szTmp[j++] = ch;
		} else {
			if (j > 0 && j < 11) {
				uint uiA = atou(szTmp);
				vctRes.push_back(uiA);
			}
			memset(szTmp, sizeof(szTmp), 0);
			j = 0;
		}
	}
	if (j > 0 && j < 11) {
		uint uiA = atou(szTmp);
		vctRes.push_back(uiA);
	}

	return 0;
}

/***
 * 获取某天的日期，日期的格式为：2010-09-02。
 * @[out]pszDate: 保存获取到的日期字符串
 * @[in]iLen: 保存日期字符串的空间长度
 * @[in]iWhichDay: 获取第几天，0为当天，1为明天，2为第三天，-1为昨天，以此类推
 * @[in]cSplit: 分隔符，形如'-'或者'/'等.默认为'-'
 * @return:
 - 0   成功
 - -1  失败
 ***/
int inline GetWhichDateStr(char *pszDate, int iLen, int iWhichDay,
		char cSplit) {
	if (NULL == pszDate || iLen < 11 || iWhichDay < 0) {
		return -1;
	}

	// 生成当前日期
	uint uiTime = (uint) time(NULL);
	if (iWhichDay < 0) {
		uiTime -= (uint) (-1 * iWhichDay) * 24 * 3600;
	} else {
		uiTime += (uint) iWhichDay * 24 * 3600;
	}

	struct tm sttm;
	my_localtime_r(uiTime, &sttm);
	snprintf(pszDate, iLen, "%04d%c%02d%c%02d", sttm.tm_year + 1900, cSplit,
			sttm.tm_mon + 1, cSplit, sttm.tm_mday);
	return 0;
}

string inline GetWhichDateStr(int iWhichDay, char cSplit) {
	char szDate[64] = { 0 };
	// 生成当前日期
	uint uiTime = (uint) time(NULL);
	if (iWhichDay < 0) {
		uiTime -= (uint) (-1 * iWhichDay) * 24 * 3600;
	} else {
		uiTime += (uint) iWhichDay * 24 * 3600;
	}

	struct tm sttm;
	my_localtime_r(uiTime, &sttm);
	snprintf(szDate, sizeof(szDate) - 1, "%04d%c%02d%c%02d",
			sttm.tm_year + 1900, cSplit, sttm.tm_mon + 1, cSplit, sttm.tm_mday);
	return szDate;
}

// 获取当前时间的字符串
void inline GetNowTimeStr(char *pszTime, int iLen) {
	if (pszTime == NULL || iLen < 1)
		return;
	memset(pszTime, 0, iLen);

	// 生成当前日期
	uint uiTime = (uint) time(NULL);
	struct tm sttm;
	my_localtime_r(uiTime, &sttm);
	snprintf(pszTime, iLen, "%04d-%02d-%02d %02d:%02d:%02d",
			sttm.tm_year + 1900, sttm.tm_mon + 1, sttm.tm_mday, sttm.tm_hour,
			sttm.tm_min, sttm.tm_sec);
	return;
}

string inline GetNowTimeStr() {
	char szDate[64] = { 0 };
	// 生成当前日期
	uint uiTime = (uint) time(NULL);
	struct tm sttm;
	my_localtime_r(uiTime, &sttm);
	snprintf(szDate, sizeof(szDate) - 1, "%04d-%02d-%02d %02d:%02d:%02d",
			sttm.tm_year + 1900, sttm.tm_mon + 1, sttm.tm_mday, sttm.tm_hour,
			sttm.tm_min, sttm.tm_sec);

	return szDate;
}

StTime inline GetNowTime() {
	return GetWhichDay(0);
}

string inline GetTimeStr(uint uiTime) {
	char szDate[64] = { 0 };
	struct tm sttm;
	my_localtime_r(uiTime, &sttm);
	snprintf(szDate, sizeof(szDate) - 1, "%04d-%02d-%02d %02d:%02d:%02d",
			sttm.tm_year + 1900, sttm.tm_mon + 1, sttm.tm_mday, sttm.tm_hour,
			sttm.tm_min, sttm.tm_sec);
	return szDate;
}

StTime inline GetWhichDay(int iDay, int iHour, int iMinute) {
	uint uiAll = (uint) time(NULL);
	if (iDay < 0) {
		uiAll -= (uint) (-1 * iDay) * 24 * 3600;
	} else {
		uiAll += (uint) iDay * 24 * 3600;
	}
	if (iHour < 0) {
		uiAll -= (uint) (-1 * iHour) * 3600;
	} else {
		uiAll += (uint) iHour * 3600;
	}
	if (iMinute < 0) {
		uiAll -= (uint) (-1 * iMinute) * 60;
	} else {
		uiAll += (uint) iMinute * 60;
	}

	StTime stTime;
	FormatStTime(uiAll, stTime);
	return stTime;
}

StTime inline GetWhichDayBaseWhichDay(uint uiTicks, int iDays) {
	if (iDays < 0) {
		uiTicks -= (uint) (-1 * iDays) * 24 * 3600;
	} else {
		uiTicks += (uint) iDays * 24 * 3600;
	}

	StTime stTime;
	FormatStTime(uiTicks, stTime);
	return stTime;
}

void inline FormatStTime(uint uiTicks, StTime &stTime) {
	stTime.uiAllTicks = uiTicks;
	struct tm sttm;
	my_localtime_r(uiTicks, &sttm);
	stTime.uiYear = sttm.tm_year + 1900;
	stTime.uiMonth = sttm.tm_mon + 1;
	stTime.uiDay = sttm.tm_mday;
	stTime.uiHour = sttm.tm_hour;
	stTime.uiMinute = sttm.tm_min;
	stTime.uiSecond = sttm.tm_sec;
	stTime.uiDayOfWeek = sttm.tm_wday;

	char szTmp[64] = { 0 };
	snprintf(szTmp, sizeof(szTmp) - 1, "%04u-%02u-%02u %02u:%02u:%02u",
			stTime.uiYear, stTime.uiMonth, stTime.uiDay, stTime.uiHour,
			stTime.uiMinute, stTime.uiSecond);
	stTime.strDateTime.assign(szTmp);

	snprintf(szTmp, sizeof(szTmp) - 1, "%04u-%02u-%02u", stTime.uiYear,
			stTime.uiMonth, stTime.uiDay);
	stTime.strDate.assign(szTmp);

	snprintf(szTmp, sizeof(szTmp) - 1, "%02u:%02u:%02u", stTime.uiHour,
			stTime.uiMinute, stTime.uiSecond);
	stTime.strTime.assign(szTmp);

	return;
}

/**
 * 在某个时间点上，获取该时间点iDays天后的时间
 * @szWhichDay: 不能是只有日期，必须带有时间。如"2013-03-03"这样的输入是非法的，可以是"2013-03-03 00:00:00"
 */
StTime inline GetWhichDayBaseWhichDay(const char *szWhichDay, int iDays) {
	uint uiAll = (uint) ConvertToTime(szWhichDay);
	return GetWhichDayBaseWhichDay(uiAll, iDays);
}

time_t inline ConvertToTime(const char *szTime) {
	struct tm tmValue;
	/*strptime(szTime,  "%Y-%m-%d %H:%M:%S", &tmValue);*/
	sscanf(szTime, "%4d-%2d-%2d %2d:%2d:%2d", &tmValue.tm_year, &tmValue.tm_mon,
			&tmValue.tm_mday, &tmValue.tm_hour, &tmValue.tm_min,
			&tmValue.tm_sec);

	tmValue.tm_year -= 1900;
	tmValue.tm_mon--;
	tmValue.tm_isdst = -1;

	return mktime(&tmValue);
}

time_t inline DateToTime(const char * szTime) {
	struct tm tmValue;
	sscanf(szTime, "%4d-%2d-%2d", &tmValue.tm_year, &tmValue.tm_mon,
			&tmValue.tm_mday);

	tmValue.tm_hour = 0;
	tmValue.tm_min = 0;
	tmValue.tm_sec = 0;

	if (tmValue.tm_year <= 1970 || tmValue.tm_mon < 1) {
		return 0;
	}
	tmValue.tm_year -= 1900;
	tmValue.tm_mon--;
	tmValue.tm_isdst = -1;

	return mktime(&tmValue);
}

/**
 * 功能等效于时间转换函数：localtime
 * 区别在于此函数可以表示的时间范围比较大，localtime的参数是int，而my_localtime_r是unsigned int
 */
int inline my_localtime_r(unsigned int timep, struct tm *result) {
	static long _timezone_ = MAX_TIME_DIFF + 1;
	const unsigned short int *ip;
	if (_timezone_ == MAX_TIME_DIFF + 1) {
		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv, &tz);
		_timezone_ = tz.tz_minuteswest * 60;
	}

	int days = (timep) / SECS_PER_DAY;
	unsigned int rem = (timep) % SECS_PER_DAY;
	rem -= _timezone_;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}
	result->tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	result->tm_min = rem / 60;
	result->tm_sec = rem % 60;
	/* January 1, 1970 was a Thursday.  */
	result->tm_wday = (4 + days) % 7;
	if (result->tm_wday < 0)
		result->tm_wday += 7;

	int y = 1970;

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))
#ifndef __isleap
	/* Nonzero if YEAR is a leap year (every 4 years,
	 * 	 *    except every 100th isn't, and every 400th is).  */
# define __isleap(year) \
		((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#endif

	while (days < 0 || days >= (__isleap (y) ? 366 : 365)) {
		/* Guess a corrected year, assuming 365 days per year.  */
		long int yg = y + days / 365 - (days % 365 < 0);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= ((yg - y) * 365 + LEAPS_THRU_END_OF(yg - 1)
				- LEAPS_THRU_END_OF(y - 1));
		y = yg;
	}
	result->tm_year = y - 1900;
	if (result->tm_year != y - 1900) {
		/* The year cannot be represented due to overflow.  */
		return -1;
	}
	result->tm_yday = days;
	ip = __mon_yday[__isleap(y)];
	for (y = 11; days < (long int) ip[y]; --y)
		continue;
	days -= ip[y];
	result->tm_mon = y;
	result->tm_mday = days + 1;

	return 0;
}

string inline getStdTimeStr(uint uiTime) {
	tm tm;
	char sBuf[64] = { 0 };
	my_localtime_r(uiTime, &tm);
	snprintf(sBuf, sizeof(sBuf), "%04d-%02d-%02d %02d:%02d:%02d",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
			tm.tm_sec);
	return sBuf;
}

string inline Char2Hex(char dec) {
	char dig1 = (dec & 0xF0) >> 4;
	char dig2 = (dec & 0x0F);
	if (0 <= dig1 && dig1 <= 9)
		dig1 += 48; //0,48inascii
	if (10 <= dig1 && dig1 <= 15)
		dig1 += 65 - 10; //a,65inascii
	if (0 <= dig2 && dig2 <= 9)
		dig2 += 48;
	if (10 <= dig2 && dig2 <= 15)
		dig2 += 65 - 10;

	string r;
	r.append(&dig1, 1);
	r.append(&dig2, 1);
	return r;
}

char inline Hex2Char(char first, char second) {
	int digit;

	digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
	digit *= 16;
	digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));

	return static_cast<char>(digit);
}

string inline EncodeURIComponent(const string &c) {
	string escaped = "";
	int max = c.length();
	for (int i = 0; i < max; i++) {
		if ((48 <= c[i] && c[i] <= 57) || //0-9
				(65 <= c[i] && c[i] <= 90) || //a-z
				(97 <= c[i] && c[i] <= 122) || //A-Z
				(c[i] == '~' || c[i] == '!' || c[i] == '*' || c[i] == '('
						|| c[i] == ')' || c[i] == '\'')) {
			escaped.append(&c[i], 1);
		} else {
			escaped.append("%");
			escaped.append(Char2Hex(c[i])); //converts char 255 to string "ff"
		}
	}
	return escaped;
}

string inline DecodeURIComponent(const string& src) {
	std::string result;
	std::string::const_iterator iter;
	char c;

	for (iter = src.begin(); iter != src.end(); ++iter) {
		switch (*iter) {
			case '+':
				result.append(1, ' ');
				break;
			case '%':
				// assume well-formed input
				c = *++iter;
				result.append(1, Hex2Char(c, *++iter));
				break;
			default:
				result.append(1, *iter);
				break;
		}
	}

	return result;
}

string inline Trim(const char *src, bool bLeft = true, bool bRight = true) {
	if (src == NULL) {
		return "";
	}

	if (!bLeft && !bRight) {
		return src;
	}

	string szDes;

	const char *Tail = src + strlen(src) - 1;
	if (bRight) {
		while (Tail >= src && isspace(*Tail)) {
			Tail--;
		}
	}

	const char *Head = src;
	if (bLeft) {
		while (*Head && isspace(*Head)) {
			Head++;
		}
	}

	if (Head <= Tail) {
		szDes.assign(Head, Tail + 1);
	} else {
		szDes = "";
	}

	return szDes;
}

}

#endif

