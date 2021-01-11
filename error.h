#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <stdexcept>

class Error: public std::runtime_error
{
    public:
        Error(int code, const std::string& path) noexcept
            : runtime_error(std::to_string(code)),
              code_(code),
              path_(path)
            {
            }

        int code() const
        {
            return code_;
        }

        std::string path() const
        {
            return path_;
        }

    private:
        int code_;
        std::string path_;
};

class BadRequest: public std::runtime_error
{
    public:
        using std::runtime_error::runtime_error;
};

class BadVerb: public std::runtime_error
{
    public:
        using std::runtime_error::runtime_error;
};

class BadVersion: public std::runtime_error
{
    public:
        using std::runtime_error::runtime_error;
};

#endif
