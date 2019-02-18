#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>

using namespace std;

#define RECV_BUFFER_SIZE    8196

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

    int recv();

    int recv_data(const char * data, const boost::system::error_code & error, const size_t bytes_transferred);
    
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
    char * _receive_buf;

};

};

#endif

