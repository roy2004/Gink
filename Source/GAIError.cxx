#include "GAIError.h"

#include <netdb.h>

#include <cassert>


namespace Gink {

GAIError::GAIError(const char *location, int errorCode, const char *string)
    : errorCode_(errorCode), description_(std::string("GAIError: ") + location + ": " + string
                                          + ": " + ::gai_strerror(errorCode))
{
    assert(errorCode != 0);
}

} // namespace Gink
