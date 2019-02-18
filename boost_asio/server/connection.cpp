
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

int Connection::recv()
{
    _socket->async_read_some(
        boost::asio::buffer(_receive_buf, RECV_BUFFER_SIZE),
        boost::bind(&Connection::recv_data, this, _receive_buf, _1, _2));
}

int Connection::recv_data(const char * data, const boost::system::error_code & error, const size_t bytes_transferred)
{
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

    return 0;
}

}

