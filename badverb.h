#ifndef BADVERB_H
#define BADVERB_H

#include <stdexcept>
#include <string>

class Badverb: public std::runtime_error
{
    public:
        Badverb(const std::string &message) noexcept
            : runtime_error(message)
            {
            }
};

#endif
