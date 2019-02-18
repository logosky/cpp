
#include "connection.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "utils.h"

using namespace std;

namespace Tcp
{

Connection::Connection(boost::asio::ip::tcp::socket * socket)
{
    _connect_status = CS_Disconnected;
    _client_port = 0;
    _socket = socket;
}

Connection::~Connection()
{
    if(_socket)
    {
        delete _socket;
        _socket = NULL;
    }
}


}

