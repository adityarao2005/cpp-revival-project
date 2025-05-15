#pragma once

#include <async/task.h>

namespace webcraft::async
{
    template <typename T>
    using task = ::async::task<T>;

    class WorkerService
    {
    private:
        friend class AsyncRuntime;
        void run();

    public:
        WorkerService(int workers);
        ~WorkerService();
        WorkerService(const WorkerService &) = delete;
        WorkerService(WorkerService &&) = delete;
        WorkerService &operator=(const WorkerService &) = delete;
        WorkerService &operator=(WorkerService &&) = delete;

        task<void> schedule();
    };
}