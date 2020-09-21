#include "tcpconnection.h"
#include <boost/bind.hpp>
#include <iostream>
#include <functional> // std::bind


using boost::asio::ip::tcp;
using boost::system::error_code;

Tcpconnection::Tcpconnection(boost::asio::io_service& io_service)
: socket_(io_service)
{
}

size_t Tcpconnection::read_complete(const error_code& error, size_t bytes)
{
    if (error) return 0;
    const std::string str = "\r\n\r\n";
    const bool found = std::search(buff_, buff_ + bytes, str.begin(), str.end()) != buff_ + bytes;
    return found ? 0 : 1;
}

void Tcpconnection::start()
{
    boost::asio::async_read (socket_, boost::asio::buffer(buff_),
        bind(&Tcpconnection::read_complete, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred),
        boost::bind(&Tcpconnection::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

void Tcpconnection::handle_read (const error_code& error,
  size_t bytes)
{
    if (error)
    {
        std::cout << "Error async_read: " << error.message() << "\n";
        return;
    }

    const std::string buff_str(buff_, bytes);
    message_ += buff_str;
    std::cout << "Message: " << message_ << "\n";
    if (bytes < 1024)
    {
        const size_t str_end_pos = message_.find('\r');
        const std::string start_str = message_.substr(0, str_end_pos);
        /*async_write*/
    }
    else
    {
        /*async_read*/
    }
}

tcp::socket& Tcpconnection::socket()
{
    return socket_;
}
