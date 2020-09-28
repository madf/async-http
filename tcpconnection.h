#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "request.h"
#include <boost/asio.hpp>
#include <string>
#include <dirent.h> //struct dirent, opendir, readdir, closedir

class Tcpconnection
    : public std::enable_shared_from_this<Tcpconnection>
{
    public:
        Tcpconnection(boost::asio::io_service& io_service, std::string& work_dir);

        void start();
        boost::asio::ip::tcp::socket& socket();

    private:
        size_t read_complete(const boost::system::error_code& error, size_t bytes);
        std::string make_index(DIR *dir, const std::string& path);
        std::string make_response(const Request& request, std::string& work_dir_);
        void handle_read(const boost::system::error_code& error, size_t bytes);

        boost::asio::ip::tcp::socket socket_;
        char buff_[1024];
        std::string message_;
        std::string work_dir_;
};

#endif
