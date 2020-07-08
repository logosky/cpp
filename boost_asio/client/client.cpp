
#include "client.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "utils.h"

using namespace std;

#define SAFE_DELETE(p) do{if(p){delete p; p = NULL;}}while(0);

namespace Demo
{

Client::Client(const string& _ip, int _port):
    _ip(_ip), _port(_port)
{
    _io = new boost::asio::io_service();
}

Client::~Client()
{
    SAFE_DELETE(_io);
}

int Client::init(bool ipv6)
{
    try
    {
        boost::asio::ip::tcp::socket * socket = new boost::asio::ip::tcp::socket(*_io);
        _connection.reset(new Connection(socket));
        if (_connection->connect(_ip, _port, ipv6))
        {
            _connection->recv();

            LOG_PRINTF("connect success, %s:%d", _ip.c_str(), _port);
        }
    }
    catch (const boost::system::system_error & e)
    {
        LOG_PRINTF("listen on %s:%d failed, %s", _ip.c_str(), _port, e.what());
        return false;
    }
    catch (...)
    {
        LOG_PRINTF("listen on %s:%d failed", _ip.c_str(), _port);
        return false;
    }
}

int Client::start()
{
    _thread = new boost::thread(boost::bind(&Client::thread_run, this));
}

int Client::thread_run()
{
    pthread_setname_np(pthread_self(), "client_run");
    _io->run();
}

int Client::send_data(const char* data, int len)
{
    _connection->send_data(data, len);
}

}

