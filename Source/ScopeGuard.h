#pragma once


#include <functional>
#include <utility>


class ScopeGuard final
{
    ScopeGuard(const ScopeGuard &) = delete;
    void operator=(const ScopeGuard &) = delete;

public:
    inline explicit ScopeGuard(const std::function<void ()> &);
    inline explicit ScopeGuard(std::function<void ()> &&);
    inline ~ScopeGuard();

    inline void appoint() noexcept;
    inline void dismiss() noexcept;

private:
    const std::function<void ()> rollback_;
    bool isEngaged_;
};


ScopeGuard::ScopeGuard(const std::function<void ()> &rollback)
    : rollback_(rollback), isEngaged_(false)
{
}


ScopeGuard::ScopeGuard(std::function<void ()> &&rollback)
    : rollback_(std::move(rollback)), isEngaged_(false)
{
}


ScopeGuard::~ScopeGuard()
{
    if (isEngaged_) {
        rollback_();
    }
}


void
ScopeGuard::appoint() noexcept
{
    isEngaged_ = true;
}


void
ScopeGuard::dismiss() noexcept
{
    isEngaged_ = false;
}
