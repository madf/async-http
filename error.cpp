#include "error.h"

Error::Error(int code, const std::string& path) noexcept
    : runtime_error(std::to_string(code)),
      code_(code),
      path_(path)
    {
    }

int Error::code() const
{
    return code_;
}

std:: string Error::path() const
{
    return path_;
}
