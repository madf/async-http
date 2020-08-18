#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>
#include <ctime>

using boost::asio::ip::tcp;
using boost::system::error_code;

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

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service, const std::string& host, const std::string& port)
      : resolver_(io_service),
        acceptor_(io_service)
    {
        namespace pls = std::placeholders;

        resolver_.async_resolve(tcp::resolver::query(host, port), bind(&tcp_server::handle_resolve, this, pls::_1, pls::_2));
    }



private:

    void handle_accept(boost::shared_ptr<tcp::socket> socket, const error_code& error)
    {
        if (!error)
        {
            std::cout << "Hello, World!" << "\n";
        }
        else
        {
            std::cout << "Error async_accept: " << error.message() << "\n";
            return;
        }
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

                        typedef boost::shared_ptr<tcp::socket> socket_ptr;
                        boost::asio::io_service io_service;
                        socket_ptr socket(new tcp::socket(io_service));

                        acceptor_.async_accept(*socket, bind(&tcp_server::handle_accept,  this, socket, boost::asio::placeholders::error));
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

    tcp::resolver resolver_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
    const std::string version = "1.3.0";
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
    tcp_server server(io_service, host, port);
    io_service.run();

    return 0;
}
