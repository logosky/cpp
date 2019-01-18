#include <hiredis/hiredis.h>
#include <iostream>
 
using namespace std;
 
int main()
{
    redisContext* c = redisConnect("127.0.0.1", 8000);
    redisReply *reply;
    if ( c->err)
    {
        redisFree(c);
        cout << "Connect to redisServer fail" << endl;
        return 1;
    }
    cout << "Connect to redisServer Success" << endl;
    redisReply* r = (redisReply*)redisCommand(c, "set test_key test_value");
    cout << r->str << endl;
    r = (redisReply*)redisCommand(c, "get test_key");
    cout << r->str << endl;
    return 0;
}
