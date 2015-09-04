#include "SystemError.h"

#include <cstring>


namespace Gink {

SystemError::SystemError(int errorNumber, const char *description)
    : errorNumber_(errorNumber), description_(description)
{
    if (errorNumber_ != 0) {
        description_.push_back(':');
        description_.push_back(' ');
        description_ += std::strerror(errorNumber_);
    }
}

} // namespace Gink
