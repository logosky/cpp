
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
    char temp_buf[128] = {0};

    memcpy(temp_buf, data, bytes_transferred);
    
    LOG_PRINTF("recv len:%d, %s", bytes_transferred, temp_buf);

    recv();
}

}

