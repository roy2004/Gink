#include "SystemError.h"

#include <cstring>


namespace Gink {

SystemError::SystemError(int errorNumber, const char *description)
    : errorNumber_(errorNumber), description_(description)
{
    description_ += ": ";
    description_ += std::strerror(errorNumber_);
}

} // namespace Gink
