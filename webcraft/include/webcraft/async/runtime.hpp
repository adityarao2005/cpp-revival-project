#pragma once

#include "awaitable.hpp"
#include <thread>
#include <optional>
#include "join_handle.hpp"
#include <webcraft/concepts.hpp>
#include <variant>
#include <algorithm>
#include <async/awaitable_resume_t.h>
#include <ranges>

namespace webcraft::async
{
#pragma region "forward declarations"

    namespace io
    {
        class IOService;
    }
    class ExecutorService;
    class TimerService;
    class AsyncRuntime;

#pragma endregion

    /// @brief Configuration class for the AsyncRuntime.
    /// This class allows the user to configure the AsyncRuntime before it is created.
    class AsyncRuntimeConfig
    {
    protected:
        friend class AsyncRuntime;
        static AsyncRuntimeConfig config;

    public:
        static void set_config(AsyncRuntimeConfig config)
        {
            AsyncRuntimeConfig::config = config;
        }

        // TODO: add more configuration options
        size_t maxWorkerThreads = 2 * std::thread::hardware_concurrency();
        size_t minWorkerThreads = std::thread::hardware_concurrency();
    };

    /// @brief Singleton-like object that manages and provides a runtime for async operations to occur.
    class AsyncRuntime
    {
    private:
#pragma region "friend classes"
        friend class io::IOService;
        friend class ExecutorService;
        friend class TimerService;

        std::unique_ptr<io::IOService> io_service;
        std::unique_ptr<ExecutorService> executor_service;
        std::unique_ptr<TimerService> timer_service;
#pragma endregion

#pragma region "constructors"
        // TODO: implement the constructors
        AsyncRuntime(AsyncRuntimeConfig config);
        AsyncRuntime(const AsyncRuntime &) = delete;
        AsyncRuntime(AsyncRuntime &&) = delete;
        AsyncRuntime &operator=(const AsyncRuntime &) = delete;
        AsyncRuntime &operator=(AsyncRuntime &&) = delete;
        ~AsyncRuntime();
#pragma endregion

    public:
#pragma region "singleton initializer"
        /// @brief Get the singleton instance of the AsyncRuntime.
        /// @return The singleton instance of the AsyncRuntime.
        static AsyncRuntime &get_instance()
        {
            // lazily initialize the instance (allow for config setup before you get the first instance)
            static AsyncRuntime instance(AsyncRuntimeConfig::config);

            return instance;
        }
#pragma endregion

#pragma region "AsyncRuntime.run"
        /// @brief Runs the asynchronous function provided and returns the result.
        /// @tparam T the type of the result of the task.
        /// @tparam ...Args the types of the arguments to the task.
        /// @param fn the function to run.
        /// @return the result of the task.
        template <typename T, typename... Args>
        T run(std::function<Task<T>(Args...)> fn, Args... args)
        {
            return run(fn(args...));
        };

        /// @brief Runs the task asynchronously and returns the result.
        /// @tparam T the type of the result of the task provided
        /// @param task the task to run.
        /// @return the result of the task.
        template <typename T>
            requires webcraft::not_same_as<T, void>
        T run(Task<T> &&task)
        {
            /// This is a bit of a hack, but we need to be able to return the result of the task
            std::optional<T> result;

            // We need to create a lambda that captures the result and returns a Task<void>
            // that sets the result when the task is done.
            auto fn = [&result](Task<T> t) -> Task<void>
            {
                result = co_await t;
            };

            // Invoke the lambda and receive the task
            auto task_fn = fn(task);
            // Run the task and waits synchronously for it to finish
            run(task_fn);

            // Check if the result is valid - This should never happen since we are using Task<T> and not Task<void>
            // but we need to check for it just in case.
            if (!result.has_value())
            {
                throw std::runtime_error("Task did not return a value");
            }
            // Get the result from the optional
            return result.value();
        }

        /// @brief Runs the task asynchronously.
        /// @param task the task to run
        // TODO: implement this function
        void run(Task<void> &&task);
#pragma endregion

#pragma region "AsyncRuntime.spawn"

        /// Spawns a task to run concurrently with the main task and returns a join handle which can be awaited for completion
        join_handle spawn(Task<void> &&task)
        {
            // Spawn task by creating join handle
            auto handle = join_handle(std::move(task));

            // spawn task internally using some kind of final awaiter
            class coro_awaiter
            {
            public:
                struct promise_type
                {
                    auto get_return_object() { return coro_awaiter(this); }
                    auto initial_suspend() { return std::suspend_always(); }
                    auto final_suspend() noexcept { return std::suspend_always(); }
                    void unhandled_exception() {}
                    void return_void() {}
                };

                explicit coro_awaiter(promise_type *p) : handle{std::coroutine_handle<promise_type>::from_promise(*p)} {}
                ~coro_awaiter()
                {
                    if (handle)
                        handle.destroy();
                }
                coro_awaiter(const coro_awaiter &) = delete;
                coro_awaiter(coro_awaiter &&) : handle{std::exchange(handle, nullptr)} {}

                bool done() const { return handle.done(); }

                std::coroutine_handle<promise_type> handle;
            };

            // Create a coroutine handle to run the task
            auto fn = [](join_handle &handle) -> coro_awaiter
            {
                co_await handle.task;
            };

            // Create the coroutine handle
            auto handle_awaiter = fn(handle);
            // Get the coroutine handle
            queue_task_resumption(handle_awaiter.handle);

            // Return the join handle
            return std::move(handle);
        }

    private:
        // internal spawn implementation
        // TODO: implement this function
        void queue_task_resumption(std::coroutine_handle<> h);

    public:
#pragma endregion

#pragma region "AsyncRuntime.join"

        /// @brief Joins all the task handles and passes a task to await their completion
        /// @tparam range the type of the ranges to join
        /// @param handles the range of join handles to join
        /// @return task
        template <std::ranges::range range>
            requires std::same_as<join_handle, std::ranges::range_value_t<range>>
        Task<void> join(range handles)
        {
            for (auto handle : handles)
                co_await handle;
        }

#pragma endregion

#pragma region "AsyncRuntime.when_all"

        /// @brief Executes all the tasks concurrently and returns the result of all tasks in the submitted order
        /// @tparam range the range of the view
        /// @param tasks
        /// @return
        template <std::ranges::range range>
            requires webcraft::not_same_as<Task<void>, std::ranges::range_value_t<range>> && Awaitable<std::ranges::range_value_t<range>>
        auto when_all(range &&tasks) -> Task<std::vector<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>>
        {
            using T = std::ranges::range_value_t<range>;
            // spawn each task and immediately collect their handles
            // join the handles and return the result
            std::vector<std::optional<T>> vec;

            co_await join(tasks | std::ranges::transform(
                                      [vec](Task<T> task)
                                      {
                                          vec.push_back({std::nullopt});
                                          size_t index = vec.size() - 1;

                                          auto fn = [vec, index](Task<T> task) -> Task<T>
                                          {
                                              vec[index] = co_await task;
                                          };

                                          return spawn(fn(task));
                                      }));

            co_return vec | std::transform([](std::optional<T> opt)
                                           { return opt.value(); }) |
                std::ranges::to<std::vector>();
        }

        template <std::ranges::range range>
            requires std::same_as<Task<void>, std::ranges::range_value_t<range>>
        Task<void> when_all(range tasks)
        {
            // spawn each task and immediately collect their handles
            // join the handles and return the result
            return join(tasks | std::views::transform([&](Task<void> t)
                                                      { return spawn(std::move(t)); }));
        }

#pragma endregion

#pragma region "AsyncRuntime.when_any"

        /// @brief Executes all the tasks concurrently and returns the first one which finishes and either cancels or discards the other tasks (once first one complete, the other tasks need not complete)
        /// @tparam ...Rets the return arguments of the tasks
        /// @param tasks the tasks to execute
        /// @return the result of the first task to finish
        template <std::ranges::range range>
            requires webcraft::not_same_as<Task<void>, std::ranges::range_value_t<range>> && Awaitable<std::ranges::range_value_t<range>>
        auto when_any(range tasks) -> Task<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>
        {
            // I know conceptually how it works but the atomic operations.... idk
            using T = ::async::awaitable_resume_t<std::ranges::range_value_t<range>>;

            std::optional<T> value;
            std::atomic<bool> flag;

            struct async_event
            {
                std::coroutine_handle<> h;

                bool await_ready() { return false; }
                void await_suspend(std::coroutine_handle<> h)
                {
                    this->h = h;
                }
                void await_resume() {}

                void set()
                {
                    h.resume();
                }
            };

            async_event ev;

            for (auto task : tasks)
            {
                auto fn = [flag, ev, value](Task<T> task) -> Task<void>
                {
                    auto val = co_await task;

                    if (flag.compare_exchange_strong(false, true, std::memory_order_relaxed))
                    {
                        value = std::move(val);
                        ev.set();
                    }
                };
                spawn(fn(task));
            }

            co_await ev;

            co_return value.value();
        }

#pragma endregion

#pragma region "AsyncRuntime.yield"
        /// @brief Yields the task to the caller and lets other tasks in the queue to resume before this one is resumed
        /// @return returns a task which can be awaited
        inline Task<void> yield()
        {
            struct yield_awaiter
            {
            public:
                AsyncRuntime &runtime;

                bool await_ready() { return false; }
                void await_suspend(std::coroutine_handle<> h)
                {
                    // queue the task for resumption
                    runtime.queue_task_resumption(h);
                }
                void await_resume() {}
            };

            co_await yield_awaiter{*this};
        }
#pragma endregion

#pragma region "asynchronous services"
        /// @brief Gets the IOService for the runtime.
        /// @return the IO service
        inline io::IOService &io_service()
        {
            return *io_service;
        }

        /// @brief Gets the ExecutorService for the runtime.
        /// @return the executor service
        inline ExecutorService &executor_service()
        {
            return *executor_service;
        }

        /// @brief Gets the TimerService for the runtime.
        /// @return the timer service
        TimerService &timer_service()
        {
            return *timer_service;
        }
#pragma endregion
    };

    template <typename T>
    Task<T> value(T &&val)
    {
        co_return std::forward<T>(val);
    }
}
