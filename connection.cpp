#include "connection.h"
#include "log.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cerrno>
#include <functional> // std::bind
#include <sys/stat.h> //stat, struct stat, open

using boost::asio::ip::tcp;
using boost::system::error_code;
using Data = std::vector<char>;

namespace pls = std::placeholders;

Data to_data(const std::string& source)
{
    return Data(source.begin(), source.end());
}

Data make_error(unsigned code, const std::string& title, const std::string& message)
{
    return to_data("HTTP/1.1 " + std::to_string(code) + " " + title + "\r\nContent-Type: text/plain\r\n\r\n"  + message + "\n");
}

std::string to_lower(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = tolower(str[i]);
    return str;
}

Data read_file(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) < 0)
        return make_error(404, "File does not exist", path + ": 404 File does not exist.");

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        if (errno == ENOENT)
            return make_error(404, "File does not exist", path + ": 404 File does not exist.");
        else if (errno == EACCES)
            return make_error(403, "File access is not allowed", path + ": 403 File access is not allowed.");
        else
            return make_error(500, "Internal server error", path + ": 500 Internal server error. " + std::string(strerror(errno)));
    }

    std::string ext = to_lower(path.substr(path.rfind(".") + 1));

    std::string header;
    if (ext == "html" || ext == "htm")
        header =  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    else
        header = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment\r\n\r\n";

    Data buff(st.st_size);
    if (read(fd, buff.data(), st.st_size) >= 0)
    {
        buff.insert(buff.begin(), header.begin(), header.end());
        close(fd);
        return buff;
    }
    else
    {
        close(fd);
        if (errno == EACCES)
            return make_error(403, "File access is not allowed", path + ": 403 File access is not allowed.");
        return make_error(500, "Internal server error.", path + ": 500 Internal server error. " + std::string(strerror(errno)));
    }
}

Connection::Connection(boost::asio::io_service& io_service, const std::string& work_dir, const std::string& outfile)
    : socket_(io_service),
      work_dir_(work_dir),
      outfile_(outfile)
{
}

size_t Connection::read_complete(const error_code& error, size_t bytes)
{
    if (error) return 0;
    const std::string str = "\r\n\r\n";
    const bool found = std::search(buff_, buff_ + bytes, str.begin(), str.end()) != buff_ + bytes;
    return found ? 0 : 1;
}

Data Connection::make_index()
{
    DIR *dir = opendir(work_dir_.c_str());
    if (dir == NULL)
        return make_error(500, "Failed to open directory", "500 Failed to open directory. " + std::string(strerror(errno)));

    std::string lines;

    for (struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        if (strcmp(".", entry->d_name) && strcmp("..", entry->d_name))
        {
            const std::string file_name = entry->d_name;

            struct stat st;
            if (stat((work_dir_ + "/" + file_name).c_str(), &st) < 0)
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
    closedir(dir);

    const std::string table_html ="<!DOCTYPE html> \
        <html> \
        <body> \
        <table border=\"1\" cellspacing=\"0\" cellpadding=\"5\"> \
        <tr><td>File name</td><td>File size</td><td>Last modification date</td></tr>" + lines + " \
        </table> \
        </body> \
        </html>";

    const std::string index =  "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n" + table_html;

    return to_data(index);
}

Data Connection::make_response(const Request& request)
{
    if (request.verb() != "GET")
        return make_error(405, "Method not allowed", "405 Method not allowed.");

    if (request.version() != "HTTP/1.1" && request.version() != "HTTP/1.0")
        return make_error(505, "HTTP Version Not Supported", "505 HTTP Version Not Supported.");

    if (request.path() != "/")
        return read_file(work_dir_ + "/" + request.path());
    else
        return make_index();
}

void Connection::start()
{
    boost::asio::async_read(socket_, boost::asio::buffer(buff_),
        std::bind(&Connection::read_complete, shared_from_this(), pls::_1, pls::_2),
        std::bind(&Connection::handle_read, shared_from_this(), pls::_1, pls::_2));
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

    message_.append(buff_, bytes);

    if (bytes < 1024)
    {
        const size_t str_end_pos = message_.find('\r');
        const std::string start_str = message_.substr(0, str_end_pos);
        try
        {
            response_ = make_response(Request(start_str));

            boost::asio::async_write(socket_, boost::asio::buffer(response_),
                boost::asio::transfer_all(),
                std::bind(&Connection::handle_write, shared_from_this(), pls::_1, pls::_2));
        }
        catch (const char* exception)
        {
            std::cerr << "Exception: " << exception << "\n";
            write_log(socket().remote_endpoint().address().to_string() + " 400 Bad request", outfile_);
            boost::asio::async_write(socket_, boost::asio::buffer(make_error(400, "Bad request", "400 Bad request.")),
                boost::asio::transfer_all(),
                std::bind(&Connection::handle_write, shared_from_this(), pls::_1, pls::_2));
        }
    }
    else
    {
        start();
    }
}

tcp::socket& Connection::socket()
{
    return socket_;
}
