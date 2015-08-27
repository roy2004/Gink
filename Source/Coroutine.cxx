#include "Coroutine.h"

#include <utility>

#include <Pixy/Runtime.h>

#include "SystemError.h"


int
FiberMain(int argc, char **argv) noexcept
{
    return CoMain(argc, argv);
}


namespace Gink {

void
CoAdd(const Coroutine &coroutine)
{
    void (*wrapper)(::uintptr_t) = [] (::uintptr_t argument) noexcept {
        Coroutine coroutine(*reinterpret_cast<Coroutine *>(argument));
        ::YieldCurrentFiber();
        coroutine();
    };

    if (!::AddAndRunFiber(wrapper, reinterpret_cast<::uintptr_t>(&coroutine))) {
        throw GINK_SYSTEM_ERROR(errno, "`::AddAndRunFiber()` failed");
    }
}


void
CoAdd(Coroutine &&coroutine)
{
    void (*wrapper)(::uintptr_t) = [] (::uintptr_t argument) noexcept {
        Coroutine coroutine(std::move(*reinterpret_cast<Coroutine *>(argument)));
        ::YieldCurrentFiber();
        coroutine();
    };

    if (!::AddAndRunFiber(wrapper, reinterpret_cast<::uintptr_t>(&coroutine))) {
        throw GINK_SYSTEM_ERROR(errno, "`::AddAndRunFiber()` failed");
    }
}


void
CoAddAndRun(const Coroutine &coroutine)
{
    void (*wrapper)(::uintptr_t) = [] (::uintptr_t argument) noexcept {
        Coroutine coroutine(*reinterpret_cast<Coroutine *>(argument));
        coroutine();
    };

    if (!::AddAndRunFiber(wrapper, reinterpret_cast<::uintptr_t>(&coroutine))) {
        throw GINK_SYSTEM_ERROR(errno, "`::AddAndRunFiber()` failed");
    }
}


void
CoAddAndRun(Coroutine &&coroutine)
{
    void (*wrapper)(::uintptr_t) = [] (::uintptr_t argument) noexcept {
        Coroutine coroutine(std::move(*reinterpret_cast<Coroutine *>(argument)));
        coroutine();
    };

    if (!::AddAndRunFiber(wrapper, reinterpret_cast<::uintptr_t>(&coroutine))) {
        throw GINK_SYSTEM_ERROR(errno, "`::AddAndRunFiber()` failed");
    }
}


void
CoYield()
{
    ::YieldCurrentFiber();
}


void
CoExit()
{
    ::ExitCurrentFiber();
}


void
CoSleep(int duration)
{
    if (!::SleepCurrentFiber(duration)) {
        throw GINK_SYSTEM_ERROR(errno, "`::SleepCurrentFiber()` failed");
    }
}

} // namespace Gink
