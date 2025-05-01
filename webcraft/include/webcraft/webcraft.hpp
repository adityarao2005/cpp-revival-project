#pragma once
#include <async/task.h>
#include <async/event_signal.h>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <utility>

struct fire_and_forget
{
    struct promise_type
    {
        void return_void() {}
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        fire_and_forget get_return_object() { return {}; }
        void unhandled_exception() { std::terminate(); }
    };
};

struct IOEvent
{
    std::function<void(int)> callback;
};

inline fire_and_forget run_task_on_io(async::task<void> task, async::event_signal &signal)
{
    co_await task;
    signal.set();
}

void run_app();