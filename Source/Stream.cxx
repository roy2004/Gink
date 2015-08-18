#include "Stream.h"

#include <cstring>
#include <climits>


namespace {

std::size_t NextPowerOfTwo(std::size_t);

} // namespace


std::size_t
Stream::read(void *buffer, std::size_t bufferSize)
{
    std::size_t dataSize = wIndex_ - rIndex_;

    if (dataSize > bufferSize) {
        dataSize = bufferSize;
    }

    if (buffer != nullptr) {
        std::memcpy(buffer, base_.data() + rIndex_, dataSize);
    }

    rIndex_ += dataSize;

    if (rIndex_ >= wIndex_ - wIndex_) {
        std::memcpy(base_.data(), base_.data() + rIndex_, wIndex_ - rIndex_);
        wIndex_ -= rIndex_;
        rIndex_ = 0;
    }

    return dataSize;
}


void
Stream::write(const void *data, std::size_t dataSize)
{
    if (base_.size() < wIndex_ + dataSize) {
        base_.resize(NextPowerOfTwo(wIndex_ + dataSize));
    }

    if (data != nullptr) {
        std::memcpy(base_.data() + wIndex_, data, dataSize);
    }

    wIndex_ += dataSize;
}


void
Stream::shrinkToFit()
{
    if (rIndex_ >= 1) {
        std::memmove(base_.data(), base_.data() + rIndex_, wIndex_ - rIndex_);
        wIndex_ -= rIndex_;
        rIndex_ = 0;
    }

    base_.erase(base_.begin() + wIndex_, base_.end());
    base_.shrink_to_fit();
}


namespace {

std::size_t
NextPowerOfTwo(std::size_t number)
{
    --number;
    int k;

    for (k = 1; k < (int)sizeof number * CHAR_BIT; k *= 2u) {
        number |= number >> k;
    }

    ++number;
    return number;
}

} // namespace
