#include "Semaphore.h"

#include <cerrno>

#include "SystemError.h"


namespace Gink {

Semaphore::Semaphore(int value, int minValue, int maxValue)
    : value_(value), minValue_(minValue), maxValue_(maxValue)
{
    if (value < minValue || value > maxValue) {
        throw GINK_SYSTEM_ERROR(EINVAL, "`Gink::Semaphore::Semaphore()` failed");
    }

    ::Event_Initialize(&notEmptyEvent_);
    ::Event_Initialize(&notFullEvent_);
}

} // namespace Gink
