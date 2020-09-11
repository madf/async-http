#include "tcpconnection.h"
#include <boost/bind.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using boost::system::error_code;

Tcpconnection::Tcpconnection(boost::asio::io_service& io_service)
: socket_(io_service)
{
}

void Tcpconnection::start()
{
    boost::asio::async_read (socket_, boost::asio::buffer(buff_),
        boost::asio::transfer_all(),
        boost::bind(&Tcpconnection::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

void Tcpconnection::handle_read (const error_code& error,
  size_t /*bytes_transferred*/)
{
    if (error)
        std::cout << "Error async_read: " << error.message() << "\n";
    return;
}

tcp::socket& Tcpconnection::socket()
{
    return socket_;
}
