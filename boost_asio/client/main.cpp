
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "utils.h"
#include "client.h"

using namespace std;
using namespace Demo;

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        LOG_PRINTF("invalid args count:%d", argc);
        return 0;
    }
    string ip = argv[1];
    int port = atoi(argv[2]);
    int ipv6 = false;
    if(argc > 3)
    {
        ipv6 = atoi(argv[3]);
    }
    LOG_PRINTF("[ipv6:%d] connect to %s:%d", ipv6, ip.c_str(), port);

    Client* client = new Client(ip, port);
    client->init(ipv6);
    client->start();

    int sequence = 1;
    char buff[128] = {0};
    TcpDataBase base;
    base.ver = 1;
    base.magic = 0xAABBCCDD;
    base.code = 1;
    base.opaque_id = 123456;
    base.data_size = 4 + 4 + 5;

    memcpy(buff, &base, sizeof(buff));
    char* buf_ptr = buff + sizeof(base);
    int* t = (int *)buf_ptr;
    *t = TDT_A;
    buf_ptr += 4;
    
    int* l = (int *)buf_ptr;
    *l = 5;
    buf_ptr += 4;
    snprintf(buf_ptr, sizeof(buff), "hello");
    
    while(1)
    {
        // snprintf(buff, sizeof(buff), "send to server:%d", sequence);

        // sequence++;

        int len = sizeof(base) + 4 + 4 + 5;
        client->send_data(buff, len);
        
        sleep(3);
        break;
    }
    
    return 0;
}
