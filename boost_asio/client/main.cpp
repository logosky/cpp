
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "utils.h"
#include "client.h"

using namespace std;
using namespace Tcp;

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        LOG_PRINTF("invalid args count:%d", argc);
        return 0;
    }
    string ip = argv[1];
    int port = atoi(argv[2]);

    LOG_PRINTF("connect to %s:%d", ip.c_str(), port);

    Client* client = new Client(ip, port);
    client->init();
    client->start();

    while(1)
    {
        sleep(1);
    }
    
    return 0;
}
