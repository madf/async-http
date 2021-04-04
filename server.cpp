#include "server.h"
#include "log.h"
#include <functional> //std::bind
#include <fstream>
#include <ctime>

using boost::asio::ip::tcp;
using boost::system::error_code;
typedef std::shared_ptr<Connection> connection_ptr;

namespace pls = std::placeholders;

Server::Server(boost::asio::io_service& io_service, const std::string& host, const std::string& port, const std::string& outfile, const std::string& work_dir)
      : io_service_(io_service),
        resolver_(io_service),
        acceptor_(io_service),
        outfile_(outfile),
        work_dir_(work_dir)
{
    resolver_.async_resolve(tcp::resolver::query(host, port), std::bind(&Server::handle_resolve, this, pls::_1, pls::_2));
}

void Server::handle_accept(connection_ptr connection, const error_code& error)
{
    if (!error)
    {
        write_log(connection->socket().remote_endpoint().address().to_string(), outfile_);

        connection->start();

        start_accept();
    }
    else
    {
        write_log("Error async_accept: " + error.message(), outfile_);
        return;
    }
}

void Server::start_accept()
{
     connection_ptr connection(std::make_shared<Connection>(io_service_, work_dir_, outfile_));
     acceptor_.async_accept(connection->socket(), std::bind(&Server::handle_accept, this, connection, pls::_1));
}

void Server::handle_resolve(const error_code& err, tcp::resolver::iterator endpoint_iterator)
{
    if (!err)
    {
        const tcp::resolver::iterator end;
        while (endpoint_iterator != end)
        {
            const tcp::endpoint ep = *endpoint_iterator;
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
                write_log("Exception handle_resolve: " + err.message(), outfile_);
            }
            endpoint_iterator++;
        }
    }
    else
    {
        write_log("Error async_resolve: " + err.message(), outfile_);
    }
}
