
#include "server.h"
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

Server::Server(const string& ip, int port, const string& ip_v6):
    _ip(ip), _port(port), _ip_v6(ip_v6)
{
    _io = new boost::asio::io_service();
    _acceptor = new boost::asio::ip::tcp::acceptor(*_io);
    _acceptor_v6 = new boost::asio::ip::tcp::acceptor(*_io);
}

Server::~Server()
{
    SAFE_DELETE(_io);
    SAFE_DELETE(_acceptor);
    SAFE_DELETE(_acceptor_v6);
}

int Server::init()
{
    LOG_PRINTF("ip:%s port:%d ip_v6:%s", _ip.c_str(), _port, _ip_v6.c_str());
    try
    {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(_ip), _port);
        _acceptor->open(endpoint.protocol());
        _acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        _acceptor->bind(endpoint);
        _acceptor->listen();

        boost::asio::ip::tcp::endpoint endpoint_v6(boost::asio::ip::address::from_string(_ip_v6), _port);
        _acceptor_v6->open(endpoint_v6.protocol());
        _acceptor_v6->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        _acceptor_v6->set_option(boost::asio::ip::v6_only(true));
        _acceptor_v6->bind(endpoint_v6);
        _acceptor_v6->listen();
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

int Server::start()
{
    post_new_accept();

    _thread = new boost::thread(boost::bind(&Server::thread_run, this));
}

void Server::post_new_accept()
{
    boost::asio::ip::tcp::socket * socket = new boost::asio::ip::tcp::socket(*_io);
    boost::shared_ptr<Connection> connection(new Connection(socket));
    _acceptor->async_accept(*socket, boost::bind(&Server::on_accept, this, connection, boost::asio::placeholders::error));

    boost::asio::ip::tcp::socket * socket_v6 = new boost::asio::ip::tcp::socket(*_io);
    boost::shared_ptr<Connection> connection_v6(new Connection(socket_v6));
    _acceptor_v6->async_accept(*socket_v6, boost::bind(&Server::on_accept, this, connection_v6, boost::asio::placeholders::error));
}

void Server::on_accept(boost::shared_ptr<Connection> connection, const boost::system::error_code& error)
{
    do
    {
        if (error)
        {
            LOG_PRINTF("on_accept error:%d, %s", error.value(), error.message().c_str());
            break;
        }

        boost::system::error_code re_ec;
        boost::asio::ip::tcp::endpoint endpoint = connection->get_socket().remote_endpoint(re_ec);
        if (re_ec)
        {
            LOG_PRINTF("on_accept remote_endpoint error:%d, %s", re_ec.value(), re_ec.message().c_str());
            break;
        }

        if(endpoint.address().is_v4())
        {
            connection->_client_ip = endpoint.address().to_v4().to_string();
        }
        else
        {
            connection->_client_ip = endpoint.address().to_v6().to_string();
        }
        connection->_client_port = endpoint.port();
        connection->_connect_status = CS_Connected;

        LOG_PRINTF("new connection, %s:%d", connection->_client_ip.c_str(), endpoint.port());

        connection->get_socket().set_option(boost::asio::socket_base::keep_alive(true));
        connection->get_socket().set_option(boost::asio::socket_base::linger(false, 5));
        connection->get_socket().set_option(boost::asio::ip::tcp::no_delay());

        _connections.insert(connection);

        connection->recv();
    } while(0);

    post_new_accept();
}

int Server::thread_run()
{
    pthread_setname_np(pthread_self(), "server_run");
    _io->run();
}

}

