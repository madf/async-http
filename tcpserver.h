#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

class Tcpserver
{
    public:
        Tcpserver(boost::asio::io_service& io_service, const std::string& host, const std::string& port, std::string& outfile);

    private:
        std::string make_daytime_string();
        std::string make_log_line(const std::string& message);
        void write_log(const std::string& message);
        void handle_write(const boost::system::error_code& error, size_t /*bytes_transferred*/);
        void handle_accept(boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code& error);
        void start_accept();
        void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

        boost::asio::io_service& io_service_;
        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ip::tcp::acceptor acceptor_;
        std::string& outfile_;
};

#endif
