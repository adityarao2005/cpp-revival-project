#pragma once

#include <async/task.h>
#include <variant>
#include <memory>
#include <functional>
#include <chrono>

namespace webcraft::async
{

    template <typename T>
    using task = ::async::task<T>;

    class IOService;
    class WorkerService;
    class TimerService;

    class AsyncRuntime;

    class AsyncRuntime
    {

    private:
        std::shared_ptr<IOService> io_service;
        std::shared_ptr<WorkerService> worker_pool;
        std::shared_ptr<TimerService> timer;

        AsyncRuntime(int workers);
        AsyncRuntime(const AsyncRuntime &) = delete;
        AsyncRuntime(AsyncRuntime &&) = delete;
        AsyncRuntime &operator=(const AsyncRuntime &) = delete;
        AsyncRuntime &operator=(AsyncRuntime &&) = delete;
        ~AsyncRuntime();

        void event_loop();
        void queue_coroutine(std::coroutine_handle<> coroutine);

    public:
        AsyncRuntime(int workers);

        task<void> yield();
        task<void> schedule_worker();

        template <typename T>
        task<T> spawn(task<T> task);

        void run(task<void> task);
        void shutdown();

        std::shared_ptr<IOService> get_io_service() const;
        std::shared_ptr<WorkerService> get_worker_pool() const;
        std::shared_ptr<TimerService> get_timer() const;

#pragma region task all and any
        template <typename... Ts>
        task<std::variant<Ts...>> any(std::tuple<task<Ts>...> tasks);

        template <typename... Ts>
        task<std::tuple<Ts...>> all(std::tuple<task<Ts>...> tasks);

        template <typename... Ts>
        task<std::variant<Ts...>> any(task<Ts> &&...tasks)
        {
            return any(std::make_tuple(std::move(tasks)...));
        }

        template <typename... Ts>
        task<std::tuple<Ts...>> all(task<Ts> &&...tasks)
        {
            return all(std::move(tasks));
        }

#pragma endregion
    };


    class TimerService
    {
    private:
        friend class AsyncRuntime;
        void run();

    public:
        template <typename Rep, typename Period>
        task<void> sleep_for(std::chrono::duration<Rep, Period> duration);

        template <typename Rep, typename Period>
        task<void> sleep_until(std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<Rep, Period>> time_point);

        template <typename Rep, typename Period>
        void run_task_after(task<void> task, std::chrono::duration<Rep, Period> duration);

        template <typename Rep, typename Period>
        void run_task_at(task<void> task, std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<Rep, Period>> time_point);

        template <typename Rep, typename Period>
        void run_task_every(task<void> task, std::chrono::duration<Rep, Period> duration);

        template <typename Rep, typename Period>
        void run_task_every(task<void> task, std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<Rep, Period>> time_point);
    };
}