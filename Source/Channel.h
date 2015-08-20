#pragma once


#include <cassert>
#include <queue>
#include <utility>

#include <Pixy/Event.h>


template <typename TYPE>
class Channel final
{
    Channel(const Channel &) = delete;
    void operator=(const Channel &) = delete;

public:
    inline Channel(int length);

    inline TYPE getMessage();
    inline void putMessage(const TYPE &);
    inline void putMessage(TYPE &&);

private:
    std::queue<TYPE> messages_;
    int length_;
    Event notEmptyEvent;
    Event notFullEvent;
};


template <typename TYPE>
Channel<TYPE>::Channel(int length)
    : length_(length)
{
    assert(length >= 0);
    Event_Initialize(&notEmptyEvent);
    Event_Initialize(&notFullEvent);
}


template <typename TYPE>
TYPE
Channel<TYPE>::getMessage()
{
    while (messages_.empty()) {
        Event_WaitFor(&notEmptyEvent);
    }

    TYPE message = std::move(messages_.front());
    messages_.pop();

    if (length_ >= 1) {
        Event_Trigger(&notFullEvent);
    }

    return std::move(message);
}


template <typename TYPE>
void
Channel<TYPE>::putMessage(const TYPE &message)
{
    if (length_ >= 1) {
        while (messages_.size() == length_) {
            Event_WaitFor(&notFullEvent);
        }
    }

    messages_.push(message);
    Event_Trigger(&notEmptyEvent);
}


template <typename TYPE>
void
Channel<TYPE>::putMessage(TYPE &&message)
{
    if (length_ >= 1) {
        while (messages_.size() == length_) {
            Event_WaitFor(&notFullEvent);
        }
    }

    messages_.push(std::move(message));
    Event_Trigger(&notEmptyEvent);
}
