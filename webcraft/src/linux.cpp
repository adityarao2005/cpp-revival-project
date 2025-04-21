#ifdef __linux__

#include "webcraft/webcraft.hpp"
#include <fmt/core.h>
#include <async/task.h>
#include <async/awaitable_get.h>

async::task<void> async_task()
{
    fmt::println("Running async task");
    co_return;
}


void run_app()
{
    async::task<void> task = async_task();
    

    fmt::println("Running on Linux");
}

#endif