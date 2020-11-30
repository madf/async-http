#include "log.h"
#include <iostream>
#include <fstream>
#include <ctime>

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

void write_log(const std::string& message, const std::string& outfile)
{
    if (!outfile.empty())
        std::ofstream(outfile, std::ios::app) << make_log_line(message);
    else
        std::cout << make_log_line(message);
}
