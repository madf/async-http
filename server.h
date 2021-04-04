#ifndef SERVER_H
#define SERVER_H

#include "connection.h"
#include <boost/asio.hpp>
#include <string>

class Server
{
    public:
        Server(boost::asio::io_service& io_service, const std::string& host, const std::string& port, const std::string& outfile, const std::string& work_dir);

    private:
        void handle_accept(std::shared_ptr<Connection> connection, const boost::system::error_code& error);
        void start_accept();
        void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

        boost::asio::io_service& io_service_;
        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ip::tcp::acceptor acceptor_;
        std::string outfile_;
        std::string work_dir_;
};

#endif
