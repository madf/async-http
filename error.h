#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <stdexcept>
#include <cstring>

class Error: public std::runtime_error
{
    public:
        Error(int code, const std::string& path) noexcept
            : runtime_error(strerror(code)),
              code_(code),
              path_(path)
            {
            }

        int code() const
        {
            return code_;
        }

        const std::string& path() const
        {
            return path_;
        }

    private:
        int code_;
        std::string path_;
};

struct BadRequest : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct BadVerb : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct BadVersion : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

#endif
