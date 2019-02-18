#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>

using namespace std;

namespace Tcp
{
enum ConnectStatus
{
    CS_Disconnected,
    CS_Connecting,
    CS_Connected
};

class Connection
{
public:
    Connection(boost::asio::ip::tcp::socket* socket);

    ~Connection();

    boost::asio::ip::tcp::socket& get_socket(void)
    {
        return *_socket;
    }

public:
    ConnectStatus _connect_status;
    
    std::string _client_ip;
    int _client_port;

private:
    boost::asio::io_service* _io;
    boost::asio::ip::tcp::socket* _socket;

};

};

#endif

