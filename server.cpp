#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

using boost::asio::ip::tcp;
using boost::system::error_code;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

void reference()
{
    std::cout << "Usage:\n";
    std::cout << "server [-a/--address <bind-address>][-d/--dir <work-dir>][-o/--out-log <log-file-name>][-h/--help][-v/--version]\n\n";

    std::cout << "-d, --dir <work-dir> - specifies the working directory with files for the server;\n";
    std::cout << "-o, --out-log <log-file-name> - specifies the file name for logging for the server;\n";
    std::cout << "-a, --address <bind-address> - can be specified in <address>[:<port>] form, where <address> can be IP-address or domain name, <port> - number;\n";
    std::cout << "-v, --version - server version;\n";
    std::cout << "-h, --help - show this text.\n";
}

std::string make_daytime_string()
{
    char buffer[80];
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    return buffer;
}

std::string make_log_line(const std::string& message)
{
    return make_daytime_string() + " " + message + "\n";
}

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service, const std::string& host, const std::string& port, std::string& outfile)
      : io_service_(io_service),
        resolver_(io_service),
        acceptor_(io_service),
        outfile_(outfile)
    {
        namespace pls = std::placeholders;

        resolver_.async_resolve(tcp::resolver::query(host, port), bind(&tcp_server::handle_resolve, this, pls::_1, pls::_2));
    }

private:
    void write_log(const std::string& message)
    {
        if (!outfile_.empty())
            std::ofstream(outfile_, std::ios::app) << make_log_line(message);
        else
            std::cout << make_log_line(message);
    }

    void handle_write(const error_code& error, size_t /*bytes_transferred*/)
    {
        if (error)
            std::cout << "Error async_write: " << error.message() << "\n";
        return;
    }

    void handle_accept(socket_ptr socket, const error_code& error)
    {
        if (!error)
        {
            write_log(socket->remote_endpoint().address().to_string());

            std::string mes = make_daytime_string();
            boost::asio::async_write(*socket, boost::asio::buffer(mes, mes.size()), boost::asio::transfer_all(),
                boost::bind(&tcp_server::handle_write, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));

            start_accept();
        }
        else
        {
            std::cout << "Error async_accept: " << error.message() << "\n";
            return;
        }
    }

    void start_accept()
    {
         socket_ptr socket(new tcp::socket(io_service_));
         acceptor_.async_accept(*socket, bind(&tcp_server::handle_accept, this, socket, boost::asio::placeholders::error));
    }

    void handle_resolve(const error_code& err, tcp::resolver::iterator endpoint_iterator)
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

    boost::asio::io_service& io_service_;
    tcp::resolver resolver_;
    tcp::acceptor acceptor_;
    std::string& outfile_;
};

int main(int argc, char* argv[])
{
    const std::string version = "1.0.0";
    std::string address;
    std::string outfile;
    std::string work_dir;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "-a" || arg == "--address")
        {
            if (i + 1 == argc)
            {
                std::cerr << arg << " needs an argument - an address.\n";
                return 1;
            }
            address = argv[++i];
        }
        else if (arg == "-d" || arg == "--dir")
        {
            if (i + 1 == argc)
            {
                std::cerr << arg << " needs an argument - a filename.\n";
                return 1;
            }
            work_dir = argv[++i];
        }
        else if (arg == "-o" || arg == "--outfile")
        {
            if (i + 1 == argc)
            {
                std::cerr << arg << " needs an argument - a filename.\n";
                return 1;
            }
            outfile = argv[++i];
        }
        else if (arg == "-v" || arg == "--version")
        {
            std::cout << "Version " << version << "\n";
            return 0;
        }
        else if (arg == "-h" || arg == "--help")
        {
            reference();
            return 0;
        }
        else
        {
            std::cerr << "Unknown command line argument\n";
            return 1;
        }
    }

    std::string port = "80";
    std::string host;
    if (!address.empty())
    {
        const size_t pos = address.find(":");
        if (pos != std::string::npos)
        {
            host = address.substr(0, pos);
            port = address.substr(pos + 1, address.length() - pos - 1);
        }
        else
        {
            host = address;
        }
    }

    boost::asio::io_service io_service;
    tcp_server server(io_service, host, port, outfile);
    io_service.run();

    return 0;
}
