/*
 *有三种方式注册毁掉函数
 *1.func_factory的构造函数
 *2.定义全局FuncRegister对象，如果用这种方式，注意至少要定义一个全局对象，防止main函数中出现空指针对象赋值
 *3.main函数中调用func_factory.reg注册
*/
#include <map>
#include <iostream>
#include <vector>
#include <string.h>
#include "comm_def.h"

using namespace std;


extern int testA(int argc , char * argv[]);
extern int testB(int argc , char * argv[]);
extern int testC(int argc , char * argv[]);

func_factory::func_factory()
{
	//1.第一种注册方式
	reg( "testA", (TEST_FUNC) &testA);
	reg( "testB", (TEST_FUNC) &testB);
	reg( "testC", (TEST_FUNC) &testC);
}

void func_factory::reg( const string & func_name ,TEST_FUNC func)
{
    if(m_funcs.find(func_name) == m_funcs.end())
    {
        m_funcs.insert(make_pair<string ,TEST_FUNC>( func_name, func));
    }
}
void func_factory::print_func() const
{
    for(map<string ,TEST_FUNC >::const_iterator citer = m_funcs.begin();
        citer != m_funcs.end();citer++)
    {
        cout << citer->first << endl;
    }
}

bool func_factory::is_valid_funcname(const string & func_name)
{
    return m_funcs.find(func_name) != m_funcs.end();
}
    
int func_factory::route(const string & func_name,int argc, char *argv[] )
{
    int ret = 0;
    map<string ,TEST_FUNC>::iterator iter = m_funcs.find(func_name);
    if(iter != m_funcs.end())
    {
        TEST_FUNC func = (TEST_FUNC)iter->second;
        ret = (*func)(argc , argv);
    }
    else
    {
        cout << "func not found" << endl;
        ret = -10;
    }
    return ret ;
}


FuncRegister::FuncRegister(const string & funcName,TEST_FUNC func)
{
    if(p_factory == NULL)
    {
        p_factory = new func_factory();
    }
    p_factory->reg(funcName, func);
}

//2.第二种注册方式
REG_FUNC(testA);
REG_FUNC(testB);
REG_FUNC(testC);
func_factory *p_factory = NULL;


int main(int argc, char *argv[])
{
    func_factory g_factory = *p_factory;
	//3.第三种注册方式
	//g_factory.reg("testA",testA);
    int ret = 0;
    if (argc < 3) 
    {
        g_factory.print_func();
        do
        {
            string func_name = "testA";
            string str_param = "1";
            while(true)
            {
                cout << "enter func name or enter help>" ;
                cout.flush();
                getline(cin,func_name);
                if(string("help") == func_name)
                {
                    g_factory.print_func();
                    continue;
                }
                if(func_name.empty() || !g_factory.is_valid_funcname(func_name))
                {
                    cout << "func_name[" << func_name << "] not found" << endl;
                    break;
                }
                cout << "enter param >";
                cout.flush();
                getline(cin,str_param);
                break;
            }
            char * sz_param[1];
            char sz_buf[256] = {0};
            memcpy(sz_buf,str_param.c_str(),str_param.size());
            sz_param[0] = sz_buf;
            //while(true)
            {
                ret = g_factory.route(func_name , 1 , sz_param);
            //    usleep(300000);
            }
            
            if(ret == 0)
            {
                cout << "test successfully" << endl << endl;
            }
            else
            {
                cout << "test failure" << endl << endl;
            }
         }
         while(true);
    }
    else
    {
        ret = g_factory.route(argv[1],argc - 2, &argv[2] );
    }
}




