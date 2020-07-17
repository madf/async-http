#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <ctime>

using boost::asio::ip::tcp;

std::string make_daytime_string()
{
    char buffer[80];
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    return buffer;
}


