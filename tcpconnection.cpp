#include "tcpconnection.h"
#include <iostream>
#include <functional> // std::bind

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace pls = std::placeholders;

Tcpconnection::Tcpconnection(boost::asio::io_service& io_service, std::string& work_dir)
    : socket_(io_service),
      work_dir_(work_dir)
{
}

size_t Tcpconnection::read_complete(const error_code& error, size_t bytes)
{
    if (error) return 0;
    const std::string str = "\r\n\r\n";
    const bool found = std::search(buff_, buff_ + bytes, str.begin(), str.end()) != buff_ + bytes;
    return found ? 0 : 1;
}

std::string Tcpconnection::make_index(DIR *dir, const std::string& path)
{
    std::string lines;

    for (struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        if (strcmp(".", entry->d_name) && strcmp("..", entry->d_name))
        {
            const std::string file_name = entry->d_name;

            struct stat st;
            if (stat((path + "/" + file_name).c_str(), &st) < 0)
            {
                lines = lines + "<tr><td>" + file_name + "</td><td>?</td><td>?</td></tr>";
            }
            else
            {
                const std::string file_date = ctime(&st.st_ctime);
                lines = lines + "<tr><td><p><a href=\"" + file_name + "\">" + file_name + "</a></p></td><td>" + std::to_string(st.st_size) + "</td><td>" + file_date + "</td></tr>";
            }
        }
    }

    const std::string table_html ="<!DOCTYPE html> \
        <html> \
        <body> \
        <table border=\"1\" cellspacing=\"0\" cellpadding=\"5\"> \
        <tr><td>File name</td><td>File size</td><td>Last modification date</td></tr>" + lines + " \
        </table> \
        </body> \
        </html>";

    const std::string index =  "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n" + table_html;

    return index;
}

std::string Tcpconnection::make_response(const Request& request, std::string& work_dir_)
{
    if (request.verb() != "GET")
        return "HTTP/1.1 405 Method not allowed\r\nContent-Type: text/plain\r\n\r\n405 Method not allowed.\n";

    if (request.version() != "HTTP/1.1" && request.version() != "HTTP/1.0")
        return "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: text/plain\r\n\r\n505 HTTP Version Not Supported.\n";

    const std::string path = work_dir_.empty() ? "." : work_dir_;
    std::string index;
    if (request.path() == "/")
    {
        DIR *dir = opendir(path.c_str());
        if (dir == NULL)
            return  "HTTP/1.1 500 Failed to open directory\r\nContent-Type: text/plain\r\n\r\n500 Failed to open directory.\n";
        index = make_index(dir, path);
        closedir(dir);
    }
    return index;
}

void Tcpconnection::start()
{
    boost::asio::async_read(socket_, boost::asio::buffer(buff_),
        bind(&Tcpconnection::read_complete, shared_from_this(), pls::_1, pls::_2),
        bind(&Tcpconnection::handle_read, shared_from_this(), pls::_1, pls::_2));
}

void Tcpconnection::handle_write(const error_code& error, size_t /*bytes_transferred*/)
{
    if (error)
    {
        std::cout << "Error async_read: " << error.message() << "\n";
        return;
    }
}

void Tcpconnection::handle_read(const error_code& error, size_t bytes)
{
    if (error)
    {
        std::cout << "Error async_read: " << error.message() << "\n";
        return;
    }

    const std::string buff_str(buff_, bytes);
    message_ += buff_str;

    if (bytes < 1024)
    {
        const size_t str_end_pos = message_.find('\r');
        const std::string start_str = message_.substr(0, str_end_pos);
        std::string msg = make_response(Request(start_str), work_dir_);

        boost::asio::async_write(socket_, boost::asio::buffer(msg),
        boost::asio::transfer_all(),
        bind(&Tcpconnection::handle_write, shared_from_this(), pls::_1, pls::_2));
    }
    else
    {
        boost::asio::async_read(socket_, boost::asio::buffer(buff_),
            bind(&Tcpconnection::read_complete, shared_from_this(), pls::_1, pls::_2),
            bind(&Tcpconnection::handle_read, shared_from_this(), pls::_1, pls::_2));
    }
}

tcp::socket& Tcpconnection::socket()
{
    return socket_;
}
