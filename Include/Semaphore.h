#pragma once


#include <climits>

#include <Pixy/Event.h>


namespace Gink {

class Semaphore final
{
    Semaphore(const Semaphore &) = delete;
    void operator=(const Semaphore &) = delete;

public:
    inline void down();
    inline void up();

    explicit Semaphore(int = INT_MIN, int = INT_MIN, int = INT_MAX);

private:
    int value_;
    int minValue_;
    int maxValue_;
    ::Event notEmptyEvent_;
    ::Event notFullEvent_;
};


void
Semaphore::down()
{
    while (value_ == minValue_) {
        ::Event_WaitFor(&notEmptyEvent_);
    }

    --value_;
    ::Event_Trigger(&notFullEvent_);
}


void
Semaphore::up()
{
    while (value_ == maxValue_) {
        ::Event_WaitFor(&notFullEvent_);
    }

    ++value_;
    ::Event_Trigger(&notEmptyEvent_);
}

} // namespace Gink
