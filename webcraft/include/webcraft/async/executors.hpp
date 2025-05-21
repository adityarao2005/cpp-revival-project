#pragma once

#include <webcraft/async/awaitable.hpp>
#include <webcraft/async/join_handle.hpp>

namespace webcraft::async
{
    class AsyncRuntime;

    class ExecutorService
    {
    private:
        friend class AsyncRuntime;
        AsyncRuntime &runtime;

#pragma region "constructors and destructors"
        ExecutorService(AsyncRuntime &runtime, int minWorkers, int maxWorkers);

    public:
        ~ExecutorService();
        ExecutorService(const ExecutorService &) = delete;
        ExecutorService(ExecutorService &&) = delete;
        ExecutorService &operator=(const ExecutorService &) = delete;
        ExecutorService &operator=(ExecutorService &&) = delete;
#pragma endregion

#pragma region "scheduling"

        /// Schedules the current coroutine onto the thread pool
        Task<void> schedule();

        /// Schedules an async function to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
        join_handle schedule(Task<void> task);
#pragma endregion
    };
}
