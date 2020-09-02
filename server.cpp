#include "tcpserver.h"
#include <boost/asio.hpp>
#include <iostream>

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
    Tcpserver server(io_service, host, port, outfile);
    io_service.run();

    return 0;
}
