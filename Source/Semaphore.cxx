#include "Semaphore.h"

#include <cerrno>
#include <stdexcept>


namespace Gink {

Semaphore::Semaphore(int value, int minValue, int maxValue)
    : value_(value), minValue_(minValue), maxValue_(maxValue)
{
    if (value < minValue || value > maxValue) {
        throw std::invalid_argument(__PRETTY_FUNCTION__);
    }

    ::Event_Initialize(&notEmptyEvent_);
    ::Event_Initialize(&notFullEvent_);
}

} // namespace Gink
