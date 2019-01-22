// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//



#pragma once

#ifndef _WIN32_WINNT        // 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501 // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif




#include <stdio.h>

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include <list>
#include <vector>

#include <iostream>
#include <string>


#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <boost/timer.hpp>

