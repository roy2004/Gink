#pragma once


#include <climits>
#include <cassert>
#include <queue>
#include <utility>

#include <Pixy/Semaphore.h>
#include <Pixy/Event.h>


namespace Gink {

template <class T>
class Channel final
{
    Channel(const Channel &) = delete;
    void operator=(const Channel &) = delete;

public:
    inline explicit Channel(int = 0);

    inline T getMessage();

    template <class U>
    inline void putMessage(U &&);

    template <class... U>
    inline void newMessage(U &&...);

private:
    const bool isUnbuffered_;
    std::queue<T> messages_;
    ::Semaphore semaphore_;
    ::Event event_;
};


template <class T>
Channel<T>::Channel(int length)
    : isUnbuffered_(length < 1)
{
    if (isUnbuffered_) {
        bool ok = ::Semaphore_Initialize(&semaphore_, 0, 0, INT_MAX);
        assert(ok);
        ::Event_Initialize(&event_);
    } else {
        bool ok = ::Semaphore_Initialize(&semaphore_, 0, 0, length);
        assert(ok);
    }
}


template <class T>
T
Channel<T>::getMessage()
{
    ::Semaphore_Down(&semaphore_);
    T message = std::move(messages_.front());
    messages_.pop();

    if (isUnbuffered_) {
        ::Event_Trigger(&event_);
    }

    return message;
}


template <class T>
template <class U>
void
Channel<T>::putMessage(U &&message)
{
    ::Semaphore_Up(&semaphore_);
    messages_.push(std::forward<U>(message));

    if (isUnbuffered_) {
        ::Event_WaitFor(&event_);
    }
}


template <class T>
template <class... U>
void
Channel<T>::newMessage(U &&...arguments)
{
    ::Semaphore_Up(&semaphore_);
    messages_.emplace(std::forward<U>(arguments)...);

    if (isUnbuffered_) {
        ::Event_WaitFor(&event_);
    }
}

} // namespace Gink
