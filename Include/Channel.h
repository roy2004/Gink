#pragma once


#include <climits>
#include <cassert>
#include <queue>
#include <utility>

#include <Pixy/Semaphore.h>


namespace Gink {

template <typename T>
class Channel final
{
    Channel(const Channel &) = delete;
    void operator=(const Channel &) = delete;

public:
    inline explicit Channel(int);

    inline T getMessage();
    inline T peekMessage();

    template <typename U>
    inline void putMessage(U &&);

    template <typename... U>
    inline void newMessage(U &&...);

private:
    ::Semaphore semaphore_;
    std::queue<T> messages_;
};


template <typename T>
Channel<T>::Channel(int length)
{
    bool ok = ::Semaphore_Initialize(&semaphore_, 0, 0, length >= 0 ? length : INT_MAX);
    assert(ok);
}


template <typename T>
T
Channel<T>::getMessage()
{
    ::Semaphore_Down(&semaphore_);
    T message = std::move(messages_.front());
    messages_.pop();
    return message;
}


template <typename T>
T
Channel<T>::peekMessage()
{
    ::Semaphore_Down(&semaphore_);
    T message = messages_.front();
    ::Semaphore_Up(&semaphore_);
    return message;
}


template <typename T>
template <typename U>
void
Channel<T>::putMessage(U &&message)
{
    ::Semaphore_Up(&semaphore_);
    messages_.push(std::forward<U>(message));
}


template <typename T>
template <typename... U>
void
Channel<T>::newMessage(U &&...arguments)
{
    ::Semaphore_Up(&semaphore_);
    messages_.emplace(std::forward<U>(arguments)...);
}

} // namespace Gink
