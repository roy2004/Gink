#include "SystemError.h"

#include <cstring>
#include <cassert>


SystemError::SystemError(const char *location, int errorNumber, const char *string)
    : errorNumber_(errorNumber), description_(std::string("SystemError: ") + location + ": "
                                              + string + ": "+ std::strerror(errorNumber))
{
    assert(errorNumber != 0);
}
