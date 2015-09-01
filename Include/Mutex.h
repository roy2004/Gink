#pragma once


#include "Semaphore.h"


namespace Gink {

class Mutex final
{
    Mutex(const Mutex &) = delete;
    void operator=(const Mutex &) = delete;

public:
    inline explicit Mutex();

    inline void lock();
    inline void unlock();

private:
    Semaphore semaphore_;
};


Mutex::Mutex()
    : semaphore_(1, 0, 1)
{
}


void
Mutex::lock()
{
    semaphore_.down();
}


void
Mutex::unlock()
{
    semaphore_.up();
}


} // namespace Gink
