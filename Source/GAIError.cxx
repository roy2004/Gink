#include "GAIError.h"

#include <netdb.h>


namespace Gink {

GAIError::GAIError(int errorCode, const char *description)
    : errorCode_(errorCode), description_(description)
{
    if (errorCode_ != 0) {
        description_.push_back(':');
        description_.push_back(' ');
        description_ += ::gai_strerror(errorCode_);
    }
}

} // namespace Gink
