
#include "connection.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <functional>
#include "utils.h"

using namespace std;

namespace Tcp
{

Connection::Connection(boost::asio::ip::tcp::socket * socket)
{
    _connect_status = CS_Disconnected;
    _client_port = 0;
    _socket = socket;
    _receive_buf = new char[RECV_BUFFER_SIZE];
}

Connection::~Connection()
{
    if(_socket)
    {
        delete _socket;
        _socket = NULL;
    }
    
    delete[] _receive_buf;
}

bool Connection::connect(const std::string & ip, const int port)
{
    _connect_status = CS_Connecting;

    boost::system::error_code ec;
    _socket->open(boost::asio::ip::tcp::v4(), ec);
    if (ec)
    {
        LOG_PRINTF("connect %s:%d m_Socket->open %d, %s", 
            ip.c_str(), 
            port, 
            ec.value(),
            ec.message().c_str());
        
        _connect_status = CS_Disconnected;
        return false;
    }

    _socket->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0), ec);
    if (ec)
    {
        LOG_PRINTF("connect %s:%d bind %d, %s", 
            ip.c_str(), 
            port, 
            ec.value(),
            ec.message().c_str());
        
        _connect_status = CS_Disconnected;
        return false;
    }

    try
    {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
        _socket->connect(endpoint, ec);
        if (ec)
        {
            LOG_PRINTF("connect %s:%d connect %d, %s", 
                ip.c_str(), 
                port,
                ec.value(), 
                ec.message().c_str());
            
            _connect_status = CS_Disconnected;
            return false;
        }
    }
    catch(...)
    {
        LOG_PRINTF("create endpoint failed, %s:%d", ip.c_str(), port);
        
        _connect_status = CS_Disconnected;
        return false;
    }

    _connect_status = CS_Connected;

    return true;
}

int Connection::recv()
{
    _socket->async_read_some(
        boost::asio::buffer(_receive_buf, RECV_BUFFER_SIZE),
        boost::bind(&Connection::recv_data, this, _receive_buf, _1, _2));
}

int Connection::recv_data(const char * data, const boost::system::error_code & error, const size_t bytes_transferred)
{
    if(error)
    {
        LOG_PRINTF("recv_data error %d, %s", error.value(), error.message().c_str());

        close();
        
        return error.value();
    }
    
    char* temp_buf = new char[128];

    memset(temp_buf, 0, 128);
    memcpy(temp_buf, data, bytes_transferred);
    
    LOG_PRINTF("recv len:%d, %s", bytes_transferred, temp_buf);

    send_data(temp_buf, strlen(temp_buf));

    recv();
}

int Connection::send_data(const char* data, int len)
{
    if (_connect_status != CS_Connected)
    {
        LOG_PRINTF("not connected:%d, send len:%d, %s", _connect_status, len, data);
        return -1;
    }

    async_write(*_socket, boost::asio::buffer(data, len),
                boost::bind(&Connection::on_sended, this, data, _1, _2));

    return 0;
}

int Connection::on_sended(const char* data, const boost::system::error_code & error,
                                   int bytes_transferred)
{
    LOG_PRINTF("send ret:%d msg:%s .data len:%d, %s", 
        error.value(), 
        error.message().c_str(), 
        bytes_transferred, 
        data);

    delete[] data;
    
    if(error)
    {
        LOG_PRINTF("on_sended error %d, %s", error.value(), error.message().c_str());

        close();
        
        return error.value();
    }

    return 0;
}

int Connection::close()
{
    boost::system::error_code ec;
    _socket->close(ec);

    LOG_PRINTF("connection close ret:%d msg:%s", 
        ec.value(), 
        ec.message().c_str());

    if (ec)
    {
        _connect_status = CS_Disconnected;
    }
}

}

