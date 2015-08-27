#pragma once


#include <cassert>
#include <queue>
#include <utility>

#include <Pixy/Event.h>


namespace Gink {

template <typename T>
class Channel final
{
    Channel(const Channel &) = delete;
    void operator=(const Channel &) = delete;

public:
    inline explicit Channel(int length);

    inline T getMessage();
    inline T &peekMessage();

    template <typename U>
    inline void putMessage(U &&);

    template <typename... U>
    inline void newMessage(U &&...);

private:
    int length_;
    std::queue<T> messages_;
    ::Event notEmptyEvent_;
    ::Event notFullEvent_;
};


template <typename T>
Channel<T>::Channel(int length)
    : length_(length)
{
    ::Event_Initialize(&notEmptyEvent_);
    ::Event_Initialize(&notFullEvent_);
}


template <typename T>
T
Channel<T>::getMessage()
{
    while (messages_.empty()) {
        ::Event_WaitFor(&notEmptyEvent_);
    }

    T message = std::move(messages_.front());
    messages_.pop();

    if (length_ >= 0) {
        ::Event_Trigger(&notFullEvent_);
    }

    return message;
}


template <typename T>
T &
Channel<T>::peekMessage()
{
    while (messages_.empty()) {
        ::Event_WaitFor(&notEmptyEvent_);
    }

    T &message = messages_.front();
    return message;
}


template <typename T>
template <typename U>
void
Channel<T>::putMessage(U &&message)
{
    if (length_ == 0) {
        messages_.push(std::forward<U>(message));
        ::Event_Trigger(&notEmptyEvent_);
        ::Event_WaitFor(&notFullEvent_);
    } else {
        if (length_ >= 1) {
            while (messages_.size() == length_) {
                ::Event_WaitFor(&notFullEvent_);
            }
        }

        messages_.push(std::forward<U>(message));
        ::Event_Trigger(&notEmptyEvent_);
    }
}


template <typename T>
template <typename... U>
void
Channel<T>::newMessage(U &&...arguments)
{
    if (length_ == 0) {
        messages_.emplace(std::forward<U>(arguments)...);
        ::Event_Trigger(&notEmptyEvent_);
        ::Event_WaitFor(&notFullEvent_);
    } else {
        if (length_ >= 1) {
            while (messages_.size() == length_) {
                ::Event_WaitFor(&notFullEvent_);
            }
        }

        messages_.emplace(std::forward<U>(arguments)...);
        ::Event_Trigger(&notEmptyEvent_);
    }
}

} // namespace Gink
