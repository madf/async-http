#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <stdexcept>

class Error: public std::runtime_error
{
    public:
        Error(int code, const std::string& path) noexcept;
        int code() const;
        std::string path() const;

    private:
        int code_;
        std::string path_;
};

#endif
