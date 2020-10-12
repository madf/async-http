#ifndef SERVER_H
#define SERVER_H

#include "tcpconnection.h"
#include <boost/asio.hpp>
#include <string>

class Server
{
    public:
        Server(boost::asio::io_service& io_service, const std::string& host, const std::string& port, std::string& outfile, std::string& work_dir);

    private:
        std::string make_daytime_string();
        std::string make_log_line(const std::string& message);
        void write_log(const std::string& message);
        void handle_accept(std::shared_ptr<Tcpconnection> connection, const boost::system::error_code& error);
        void start_accept();
        void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

        boost::asio::io_service& io_service_;
        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ip::tcp::acceptor acceptor_;
        std::string outfile_;
        std::string work_dir_;
        char buff_[1024];
};

#endif
