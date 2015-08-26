#include "GAIError.h"

#include <netdb.h>

#include <cassert>


namespace Gink {

GAIError::GAIError(int errorCode, const char *description)
    : errorCode_(errorCode), description_(description)
{
    assert(errorCode != 0);
    description_ += ::gai_strerror(errorCode_);
}

} // namespace Gink
