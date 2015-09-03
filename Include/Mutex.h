#pragma once


#include <cassert>

#include <Pixy/Semaphore.h>


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
    ::Semaphore semaphore_;
};


Mutex::Mutex()
{
    bool ok = ::Semaphore_Initialize(&semaphore_, 1, 0, 1);
    assert(ok);
}


void
Mutex::lock()
{
    ::Semaphore_Down(&semaphore_);
}


void
Mutex::unlock()
{
    ::Semaphore_Up(&semaphore_);
}


} // namespace Gink
