#include "SystemError.h"

#include <cstring>
#include <cassert>


namespace Gink {

SystemError::SystemError(int errorNumber, const char *description)
    : errorNumber_(errorNumber), description_(description)
{
    assert(errorNumber != 0);
    description_ += std::strerror(errorNumber_);
}

} // namespace Gink
