#ifndef LOG_H
#define LOG_H

#include <boost/asio.hpp>
#include <string>

void write_log(const std::string& message, const std::string& outfile);
void write_log(const boost::asio::ip::tcp::socket& socket, const std::string& message, const std::string& outfile);

#endif
