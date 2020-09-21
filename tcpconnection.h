#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <boost/asio.hpp>
#include <string>

class Tcpconnection
{
    public:
        Tcpconnection(boost::asio::io_service& io_service);

        void start();
        boost::asio::ip::tcp::socket& socket();

    private:
        size_t read_complete(const boost::system::error_code& error, size_t bytes);

        void handle_read(const boost::system::error_code& error,
          size_t bytes);

        boost::asio::ip::tcp::socket socket_;
        char buff_[1024];
        std::string message_;
};

#endif
