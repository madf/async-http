#ifndef BADVERSION_H
#define BADVERSION_H

#include <stdexcept>
#include <string>

class Badversion: public std::runtime_error
{
    public:
        Badversion(const std::string &message) noexcept
            : runtime_error(message)
            {
            }
};

#endif
