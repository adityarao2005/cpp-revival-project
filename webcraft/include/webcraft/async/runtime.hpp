#pragma once

#include <webcraft/async/awaitable.hpp>
#include <thread>
#include <optional>
#include <webcraft/async/join_handle.hpp>
#include <webcraft/concepts.hpp>
#include <variant>
#include <algorithm>
#include <async/awaitable_resume_t.h>
#include <ranges>
#include <webcraft/async/config.hpp>

namespace webcraft::async
{
#pragma region "forward declarations"

    namespace io
    {
        class io_service;
    }
    class executor_service;
    class timer_service;
    class async_runtime;

#pragma endregion

    /// @brief Singleton-like object that manages and provides a runtime for async operations to occur.
    class async_runtime
    {
    private:
#pragma region "friend classes"
        friend class io::io_service;
        friend class executor_service;
        friend class timer_service;

        std::unique_ptr<io::io_service> io_svc;
        std::unique_ptr<executor_service> executor_svc;
        std::unique_ptr<timer_service> timer_svc;
#pragma endregion

#pragma region "constructors"
        // TODO: implement the constructors in cpp file
        async_runtime(std::unique_ptr<async_runtime_config> config);
        async_runtime(const async_runtime &) = delete;
        async_runtime(async_runtime &&) = delete;
        async_runtime &operator=(const async_runtime &) = delete;
        async_runtime &operator=(async_runtime &&) = delete;
        ~async_runtime();
#pragma endregion

    public:
#pragma region "singleton initializer"
        /// @brief Get the singleton instance of the async_runtime.
        /// @return The singleton instance of the async_runtime.
        static async_runtime &get_instance();
        // {
        //     // lazily initialize the instance (allow for config setup before you get the first instance)
        //     static async_runtime instance(*(async_runtime_config::config));

        //     return instance;
        // }
#pragma endregion

#pragma region "AsyncRuntime.run"
        /// @brief Runs the asynchronous function provided and returns the result.
        /// @tparam T the type of the result of the task.
        /// @tparam ...Args the types of the arguments to the task.
        /// @param fn the function to run.
        /// @return the result of the task.
        template <class Fn, class... Args>
            requires awaitable<std::invoke_result_t<Fn, Args...>> && std::is_void_v<::async::awaitable_resume_t<std::remove_cvref_t<std::invoke_result_t<Fn, Args...>>>>
        void run_async(Fn &&fn, Args &&...args)
        {
            auto task = std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);

            // If the task is a task<void>, we can just run it
            run(std::move(task));
        };

        /// @brief Runs the asynchronous function provided and returns the result.
        /// @tparam T the type of the result of the task.
        /// @tparam ...Args the types of the arguments to the task.
        /// @param fn the function to run.
        /// @return the result of the task.
        template <class Fn, class... Args>
            requires awaitable<std::invoke_result_t<Fn, Args...>> && webcraft::not_true<std::is_void_v<::async::awaitable_resume_t<std::remove_cvref_t<std::invoke_result_t<Fn, Args...>>>>>
        auto run_async(Fn &&fn, Args &&...args)
        {
            auto task = std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);

            // If the task is a task<void>, we can just run it
            return run(std::move(task));
        };

        /// @brief Runs the task asynchronously and returns the result.
        /// @tparam T the type of the result of the task provided
        /// @param task the task to run.
        /// @return the result of the task.
        template <typename T>
            requires webcraft::not_same_as<T, void>
        T run(task<T> &&t)
        {
            /// This is a bit of a hack, but we need to be able to return the result of the task
            std::optional<T> result;

            // We need to create a lambda that captures the result and returns a task<void>
            // that sets the result when the task is done.
            auto fn = [&result](task<T> t) -> task<void>
            {
                result = co_await t;
            };

            // Invoke the lambda and receive the task
            auto task_fn = fn(t);
            // Run the task and waits synchronously for it to finish
            run(task_fn);

            // Check if the result is valid - This should never happen since we are using task<T> and not task<void>
            // but we need to check for it just in case.
            if (!result.has_value())
            {
                throw std::runtime_error("task did not return a value");
            }
            // Get the result from the optional
            return result.value();
        }

        /// @brief Runs the task asynchronously.
        /// @param task the task to run
        // TODO: implement this function in cpp file
        void run(task<void> &&t);
#pragma endregion

#pragma region "AsyncRuntime.spawn"

        /// @brief Spawns a task to run concurrently with the main task and returns a join handle which can be awaited for completion
        /// @param task the task to be spawned
        /// @return the join handle for the task
        join_handle spawn(task<void> &&t)
        {
            // Spawn task by creating join handle
            auto handle = join_handle(std::move(t));

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
                co_await handle.t;
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
        // TODO: implement this function in cpp file
        void queue_task_resumption(std::coroutine_handle<> h);

    public:
#pragma endregion

#pragma region "AsyncRuntime.join"

        /// @brief Joins all the task handles and passes a task to await their completion
        /// @tparam range the type of the ranges to join
        /// @param handles the range of join handles to join
        /// @return task
        template <std::ranges::input_range range>
            requires std::same_as<join_handle, std::ranges::range_value_t<range>>
        task<void> join(range handles)
        {
            for (auto handle : handles)
                co_await handle;
        }

#pragma endregion

#pragma region "AsyncRuntime.when_all"

        /// @brief Executes all the tasks concurrently and returns the result of all tasks in the submitted order
        /// @tparam range the range of the view
        /// @param tasks the tasks to execute
        /// @return the result of all tasks in the submitted order
        template <std::ranges::input_range range>
            requires webcraft::not_same_as<task<void>, std::ranges::range_value_t<range>> && awaitable<std::ranges::range_value_t<range>>
        auto when_all(range &&tasks) -> task<std::vector<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>>
        {
            using T = std::ranges::range_value_t<range>;
            // spawn each task and immediately collect their handles
            // join the handles and return the result
            std::vector<std::optional<T>> vec;

            co_await join(tasks | std::views::transform(
                                      [vec](task<T> t)
                                      {
                                          vec.push_back({std::nullopt});
                                          size_t index = vec.size() - 1;

                                          auto fn = [vec, index](task<T> t) -> task<T>
                                          {
                                              vec[index] = co_await t;
                                          };

                                          return spawn(fn(t));
                                      }));

            auto pipe = vec | std::views::transform([](std::optional<T> opt)
                                                    { return opt.value(); });
            std::vector<T> out;
            out.reserve(std::ranges::distance(pipe));
            std::ranges::copy(pipe, std::back_inserter(out));
            co_return out;
        }

        /// @brief Executes all the tasks concurrently
        /// @tparam range the range of the view
        /// @param tasks the tasks to execute
        /// @return an awaitable
        template <std::ranges::input_range range>
            requires std::same_as<task<void>, std::ranges::range_value_t<range>>
        task<void> when_all(range tasks)
        {
            // spawn each task and immediately collect their handles
            // join the handles and return the result
            return join(tasks | std::views::transform([&](task<void> t)
                                                      { return spawn(std::move(t)); }));
        }

#pragma endregion

#pragma region "AsyncRuntime.when_any"

        /// @brief Executes all the tasks concurrently and returns the first one which finishes and either cancels or discards the other tasks (once first one complete, the other tasks need not complete)
        /// @tparam ...Rets the return arguments of the tasks
        /// @param tasks the tasks to execute
        /// @return the result of the first task to finish
        template <std::ranges::input_range range>
            requires webcraft::not_same_as<task<void>, std::ranges::range_value_t<range>> && awaitable<std::ranges::range_value_t<range>>
        auto when_any(range tasks) -> task<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>
        {
            // I know conceptually how it works but the atomic operations.... idk
            using T = ::async::awaitable_resume_t<std::ranges::range_value_t<range>>;

            std::optional<T> value;
            std::atomic<bool> flag;

            // Create an event which will resume the coroutine when the event is set
            struct async_event
            {
                std::coroutine_handle<> h;

                constexpr bool await_ready() { return false; }
                constexpr void await_suspend(std::coroutine_handle<> h)
                {
                    this->h = h;
                }
                constexpr void await_resume() {}

                void set()
                {
                    // resumes the coroutine
                    h.resume();
                }
            };

            // create teh event
            async_event ev;

            // go through all the tasks and spawn them
            for (auto t : tasks)
            {
                auto fn = [flag, ev, value](task<T> t) mutable -> task<void>
                {
                    auto val = co_await t;

                    // only set the event if the flag is not set
                    bool check = false;
                    if (flag.compare_exchange_strong(check, true, std::memory_order_relaxed))
                    {
                        value = std::move(val);
                        ev.set();
                    }
                };
                // spawn the task
                spawn(fn(t));
            }

            // wait for the event to be set
            co_await ev;

            // return the value
            co_return value.value();
        }

#pragma endregion

#pragma region "AsyncRuntime.yield"
        /// @brief Yields the task to the caller and lets other tasks in the queue to resume before this one is resumed
        /// @return returns a task which can be awaited
        inline task<void> yield()
        {
            struct yield_awaiter
            {
            public:
                async_runtime &runtime;

                constexpr bool await_ready() { return false; }
                void await_suspend(std::coroutine_handle<> h)
                {
                    // queue the task for resumption
                    runtime.queue_task_resumption(h);
                }
                constexpr void await_resume() {}
            };

            co_await yield_awaiter{*this};
        }
#pragma endregion

#pragma region "asynchronous services"
        /// @brief Gets the io_service for the runtime.
        /// @return the IO service
        io::io_service &get_io_service();

        /// @brief Gets the executor_service for the runtime. Helps run tasks in parallel
        /// @return the executor service
        executor_service &get_executor_service();

        /// @brief Gets the timer_service for the runtime.
        /// @return the timer service
        timer_service &get_timer_service();
#pragma endregion
    };

    template <typename T>
    task<T> value_of(T &&val)
    {
        co_return std::forward<T>(val);
    }
}

#include <webcraft/async/executors.hpp>
#include <webcraft/async/timer_service.hpp>