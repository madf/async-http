#ifndef BADVERSION_H
#define BADVERSION_H

#include <stdexcept>
#include <string>

class BadVersion: public std::runtime_error
{
    public:
        using std::runtime_error::runtime_error;
};

#endif
