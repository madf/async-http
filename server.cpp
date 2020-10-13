#include "server.h"
#include "tcpconnection.h"
#include "request.h"
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>
#include <ctime>

using boost::asio::ip::tcp;
using boost::system::error_code;
typedef std::shared_ptr<Connection> connection_ptr;

Server::Server(boost::asio::io_service& io_service, const std::string& host, const std::string& port, std::string& outfile, std::string& work_dir)
      : io_service_(io_service),
        resolver_(io_service),
        acceptor_(io_service),
        outfile_(outfile),
        work_dir_(work_dir)
{
    namespace pls = std::placeholders;

    resolver_.async_resolve(tcp::resolver::query(host, port), bind(&Server::handle_resolve, this, pls::_1, pls::_2));
}

std::string Server::make_daytime_string()
{
    char buffer[80];
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    return buffer;
}

std::string Server::make_log_line(const std::string& message)
{
    return make_daytime_string() + " " + message + "\n";
}

void Server::write_log(const std::string& message)
{
    if (!outfile_.empty())
        std::ofstream(outfile_, std::ios::app) << make_log_line(message);
    else
        std::cout << make_log_line(message);
}

void Server::handle_accept(connection_ptr connection, const error_code& error)
{
    if (!error)
    {
        write_log(connection->socket().remote_endpoint().address().to_string());

        connection->start();

        start_accept();
    }
    else
    {
        std::cout << "Error async_accept: " << error.message() << "\n";
        return;
    }
}

void Server::start_accept()
{
     connection_ptr connection(new Connection(io_service_, work_dir_));
     acceptor_.async_accept(connection->socket(), bind(&Server::handle_accept, this, connection, boost::asio::placeholders::error));
}

void Server::handle_resolve(const error_code& err, tcp::resolver::iterator endpoint_iterator)
{
    if (!err)
    {
        try
        {
            tcp::resolver::iterator end;
            while (endpoint_iterator != end)
            {
                tcp::endpoint ep = *endpoint_iterator;
                try
                {
                    acceptor_.open(ep.protocol());
                    acceptor_.bind(ep);
                    acceptor_.listen();

                    start_accept();

                    break;
                }
                catch (const std::exception& e)
                {
                    acceptor_.close();
                    std::cout << "Exception: " << std::string(e.what()) << "\n";
                }
                endpoint_iterator++;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception: " << std::string(e.what()) << "\n";
        }
    }
    else
    {
        std::cout << "Error: " << err.message() << "\n";
    }
}
