#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "connection.h"

using namespace std;

namespace Tcp
{
class Client
{
public:
    Client(const string& ip, int port);

    ~Client();

    int init();
    
    int start();
    
    int thread_run();

    int send_data(const char* data, int len);
private:
    string _ip;
    int _port;
    
    boost::asio::io_service* _io;
    boost::asio::ip::tcp::acceptor* _acceptor;

    boost::shared_ptr<Connection> _connection;
    
    boost::thread * _thread;
    
};

};

#endif

