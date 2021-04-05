#ifndef CONNECTION_H
#define CONNECTION_H

#include "request.h"
#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <dirent.h> //struct dirent, opendir, readdir, closedir

using Data = std::vector<char>;

class Connection
    : public std::enable_shared_from_this<Connection>
{
    public:
        Connection(boost::asio::io_service& io_service, const std::string& work_dir, const std::string& outfile);

        void start();
        boost::asio::ip::tcp::socket& socket();

    private:
        size_t read_complete(const boost::system::error_code& error, size_t bytes);
        Data make_index();
        Data make_response(const Request& request);
        void handle_read(const boost::system::error_code& error, size_t bytes);
        void handle_write(const boost::system::error_code& error, size_t /*bytes_transferred*/);
        void handle_exception(unsigned code, const std::string& title, const std::string& message);

        boost::asio::ip::tcp::socket socket_;
        char buff_[1024];
        std::string message_;
        std::string work_dir_;
        Data response_;
        std::string outfile_;
};

#endif
