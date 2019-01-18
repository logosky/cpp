#ifndef __COMM_UNIT_DEF__
#define __COMM_UNIT_DEF__

#include <string>

using namespace std;

typedef int (*TEST_FUNC)(int argc, char *argv[]);

class func_factory
{
public:
    func_factory();
    
    void reg( const string & func_name ,TEST_FUNC func);
    
    void print_func() const;
    
    bool is_valid_funcname(const string & func_name);
    
    int route(const string & func_name,int argc, char *argv[]);
    
protected :
    map<string ,TEST_FUNC > m_funcs;
};  

extern func_factory * p_factory;


class FuncRegister
{
public:
    FuncRegister(const string & funcName,TEST_FUNC func);
};


#define REG_FUNC(func)    \
    FuncRegister obj_##func(#func,(TEST_FUNC)&func);


#endif


