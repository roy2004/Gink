#include "GAIError.h"

#include <netdb.h>


namespace Gink {

GAIError::GAIError(int errorCode, const char *description)
    : errorCode_(errorCode), description_(description)
{
    description_ += ": ";
    description_ += ::gai_strerror(errorCode_);
}

} // namespace Gink
