#pragma once


#include <cstddef>
#include <vector>


class Stream final
{
    explicit Stream(const Stream &) = delete;
    void operator=(const Stream &) = delete;

public:
    inline explicit Stream();
    inline ~Stream();

    inline const void *getData();
    inline std::size_t getDataSize();
    inline void *getBuffer();
    inline std::size_t getBufferSize();

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
Stream::getData()
{
    return base_.data() + rIndex_;
}


std::size_t
Stream::getDataSize()
{
    return wIndex_ - rIndex_;
}


void *
Stream::getBuffer()
{
    return base_.data() + wIndex_;
}


std::size_t
Stream::getBufferSize()
{
    return base_.size() - wIndex_;
}
