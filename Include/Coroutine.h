#pragma once


#include <functional>


int CoMain(int, char **);


namespace Gink {

using Coroutine = std::function<void ()>;


void CoAdd(const Coroutine &);
void CoAdd(Coroutine &&);
void CoAddAndRun(const Coroutine &);
void CoAddAndRun(Coroutine &&);
void CoYield();
[[noreturn]] void CoExit();
void CoSleep(int);

} // namespace Gink
