#pragma once


#include <functional>


using Coroutine = std::function<void ()>;


int CoMain(int, char **);
void CoAdd(const Coroutine &);
void CoAdd(Coroutine &&);
void CoAddAndRun(const Coroutine &);
void CoAddAndRun(Coroutine &&);
void CoYield();
[[noreturn]] void CoExit();
void CoSleep(int);
