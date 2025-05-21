#pragma once
#include <webcraft/async/awaitable.hpp>
#include <async/task.h>
#include <webcraft/concepts.hpp>

namespace webcraft::async
{
    // TODO: Implement this somehow - think of something and make it look like its not just a ::async::task object
    template <typename T>
    class join_handle
    {
    public:
        Task<T> join();
    };

};