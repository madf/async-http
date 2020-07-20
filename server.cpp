#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <ctime>

using boost::asio::ip::tcp;

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


