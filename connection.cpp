#include "connection.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cerrno>
#include <functional> // std::bind
#include <sys/stat.h> //stat, struct stat, open

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace pls = std::placeholders;

Connection::Connection(boost::asio::io_service& io_service, const std::string& work_dir)
    : socket_(io_service),
      work_dir_(work_dir)
{
}

size_t Connection::read_complete(const error_code& error, size_t bytes)
{
    if (error) return 0;
    const std::string str = "\r\n\r\n";
    const bool found = std::search(buff_, buff_ + bytes, str.begin(), str.end()) != buff_ + bytes;
    return found ? 0 : 1;
}

Data Connection::toData(const std::string& source)
{
    return Data(source.begin(), source.end());
}


Data Connection::read_file(const Request& request, const std::string& path)
{
    int fd = open((path + "/" + request.path()).c_str(), O_RDONLY);

    if (fd == -1)
    {
        if (errno == ENOENT)
            return toData("HTTP/1.1 404 File does not exist\r\nContent-Type: text/plain\r\n\r\n404 File does not exist.\n");
        else if (errno == EACCES)
            return toData("HTTP/1.1 403 File access not allowed\r\nContent-Type: text/plain\r\n\r\n403 File access not allowed.\n");
        else
            return toData("HTTP/1.1 500 File open error\r\nContent-Type: text/plain\r\n\r\n500 File open error." + std::string(strerror(errno)) + "\n");
    }

    std::string ext = request.path().substr(request.path().find(".") + 1);

    for (size_t i = 0; i < ext.length(); i++)
        ext[i] = tolower(ext[i]);

    std::string header;

    if (ext == "html" || ext == "htm")
        header =  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    else
        header = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment\r\n\r\n";

    struct stat st;
    if (stat((path + "/" + request.path()).c_str(), &st) < 0)
        return toData("HTTP/1.1 404 File does not exist\r\nContent-Type: text/plain\r\n\r\n404 File does not exist.\n");

    Data buff(st.st_size);
    if (read(fd, buff.data(), st.st_size) >= 0)
    {
        buff.insert(buff.begin(), header.begin(), header.end());
        return buff;
    }
    else
    {
        if (errno == EACCES)
            return toData("HTTP/1.1 403 File access not allowed.\r\nContent-Type: text/plain\r\n\r\n403 File access not allowed.\n");
        else
            return toData("HTTP/1.1 500 The file descriptor is invalid.\r\nContent-Type: text/plain\r\n\r\n500 The file descriptor is invalid." + std::string(strerror(errno)) + "\n");
    }
}

Data Connection::make_index(DIR *dir, const std::string& path)
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

    return toData(index);
}

Data Connection::make_response(const Request& request, const std::string& work_dir_)
{
    if (request.verb() != "GET")
        return toData("HTTP/1.1 405 Method not allowed\r\nContent-Type: text/plain\r\n\r\n405 Method not allowed.\n");

    if (request.version() != "HTTP/1.1" && request.version() != "HTTP/1.0")
        return toData("HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: text/plain\r\n\r\n505 HTTP Version Not Supported.\n");

    const std::string path = work_dir_.empty() ? "." : work_dir_;

    if (request.path() != "/")
    {
        return read_file(request.path(), path);
    }
    else
    {
        DIR *dir = opendir(path.c_str());
        if (dir == NULL)
            return toData("HTTP/1.1 500 Failed to open directory\r\nContent-Type: text/plain\r\n\r\n500 Failed to open directory.\n");
        Data index = make_index(dir, path);
        closedir(dir);
        return index;
    }
}

void Connection::start()
{
    boost::asio::async_read(socket_, boost::asio::buffer(buff_),
        bind(&Connection::read_complete, shared_from_this(), pls::_1, pls::_2),
        bind(&Connection::handle_read, shared_from_this(), pls::_1, pls::_2));
}

void Connection::handle_write(const error_code& error, size_t /*bytes_transferred*/)
{
    if (error)
    {
        std::cout << "Error async_write: " << error.message() << "\n";
        return;
    }
}

void Connection::handle_read(const error_code& error, size_t bytes)
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
        msg_ = make_response(Request(start_str), work_dir_);

        boost::asio::async_write(socket_, boost::asio::buffer(msg_, msg_.size()),
        boost::asio::transfer_all(),
        bind(&Connection::handle_write, shared_from_this(), pls::_1, pls::_2));
    }
    else
    {
        boost::asio::async_read(socket_, boost::asio::buffer(buff_),
            bind(&Connection::read_complete, shared_from_this(), pls::_1, pls::_2),
            bind(&Connection::handle_read, shared_from_this(), pls::_1, pls::_2));
    }
}

tcp::socket& Connection::socket()
{
    return socket_;
}
