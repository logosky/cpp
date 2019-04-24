#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "connection.h"

using namespace std;

namespace Tcp
{
class Server
{
public:
    Server(const string& ip, int port);

    ~Server();

    int init();
    
    int start();

    void post_new_accept();

    void on_accept(boost::shared_ptr<Connection> connection, const boost::system::error_code& error);
    
    int thread_run();

private:
    string _ip;
    int _port;
    
    boost::asio::io_service* _io;
    boost::asio::ip::tcp::acceptor* _acceptor;
    
    boost::thread * _thread;

    set< boost::shared_ptr<Connection> > _connections;
    
};

};

#endif

