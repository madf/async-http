#ifndef BADVERSION_H
#define BADVERSION_H

#include <stdexcept>
#include <string>

class Badversion: public std::runtime_error
{
    public:
        using std::runtime_error::runtime_error;
};

#endif
