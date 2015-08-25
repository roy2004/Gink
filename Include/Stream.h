#pragma once


#include <cstddef>
#include <vector>


namespace Gink {

class Stream final
{
    Stream(const Stream &) = delete;
    void operator=(const Stream &) = delete;

public:
    explicit inline Stream();
    inline ~Stream();

    inline const void *getData() const;
    inline void *getData();
    inline std::size_t getDataSize() const;
    inline void *getBuffer();
    inline std::size_t getBufferSize() const;

    std::size_t read(void *, std::size_t);
    void write(const void *, std::size_t);
    void shrinkToFit();

private:
    std::vector<char> base_;
    std::ptrdiff_t rIndex_;
    std::ptrdiff_t wIndex_;
};


Stream::Stream()
    : rIndex_(0), wIndex_(0)
{
}


Stream::~Stream()
{
}


const void *
Stream::getData() const
{
    return base_.data() + rIndex_;
}


void *
Stream::getData()
{
    return base_.data() + rIndex_;
}


std::size_t
Stream::getDataSize() const
{
    return wIndex_ - rIndex_;
}


void *
Stream::getBuffer()
{
    return base_.data() + wIndex_;
}


std::size_t
Stream::getBufferSize() const
{
    return base_.size() - wIndex_;
}

} // namespace Gink
