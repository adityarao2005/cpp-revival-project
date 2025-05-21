#pragma once

#include "awaitable.hpp"
#include <thread>
#include <optional>
#include "join_handle.hpp"
#include <webcraft/concepts.hpp>
#include <variant>

namespace webcraft::async
{
#pragma region "forward declarations"

    class IOService;
    class ExecutorService;
    class TimerService;
    class AsyncRuntime;

#pragma endregion

    class AsyncRuntimeConfig
    {
    private:
        // Let them have the option to have a cached thread pool
        size_t maxCachedThreads = 2 * std::thread::hardware_concurrency();
        size_t workerThreads = std::thread::hardware_concurrency();

        friend class AsyncRuntime;
        static AsyncRuntimeConfig config;

    public:
        static void set_config(AsyncRuntimeConfig config)
        {
            AsyncRuntimeConfig::config = config;
        }
    };

    /// @brief Singleton-like object that manages and provides a runtime for async operations to occur.
    class AsyncRuntime
    {
    private:
#pragma region "friend classes"
        friend class IOService;
        friend class ExecutorService;
        friend class TimerService;
#pragma endregion

#pragma region "constructors"
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

            // TODO: configure the runtime here
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
        void run(Task<void> &&task);
#pragma endregion

#pragma region "AsyncRuntime.spawn"

        /// Spawns a task to run concurrently with the main task and returns a join handle which can be awaited for completion
        template <typename T, typename... Args>
        join_handle<T> spawn(std::function<Task<T>(Args...)> fn, Args... args)
        {
            return spawn(fn(args...));
        }

        // TODO: Solidify this once I'm done figuring out how join_handle should be implemented
        /// Spawns a task to run concurrently with the main task and returns a join handle which can be awaited for completion
        template <typename T>
            requires webcraft::not_same_as<T, void>
        join_handle<T> spawn(Task<T> &&task);

        /// Spawns a task to run concurrently with the main task and returns a join handle which can be awaited for completion
        join_handle<void> spawn(Task<void> &&task);

#pragma endregion

#pragma region "AsyncRuntime.join"

        /// Joins all the task handles and passes a task to await their completion
        template <typename... Rets>
        Task<std::tuple<Rets...>> join(std::tuple<join_handle<Rets>...> handles);

        /// Joins all the task handles and passes a task to await their completion
        template <typename... Rets>
        Task<std::tuple<Rets...>> join(join_handle<Rets>... handles)
        {
            return join(std::make_tuple(handles...));
        }

#pragma endregion

#pragma region "AsyncRuntime.when_all"

        /// @brief Executes all the tasks concurrently and returns the result of all tasks in the submitted order
        /// @tparam ...Rets the return types of the tasks
        /// @param ...tasks the tasks to execute
        /// @return the result of all tasks in the submitted order
        template <typename... Rets>
        Task<std::tuple<Rets...>> when_all(Task<Rets> &&...tasks)
        {
            // spawn each task and immediately collect their handles
            auto handles = std::make_tuple(
                spawn(std::move(tasks))...);

            // join the handles and return the result
            return join(std::move(handles));
        }

#pragma endregion

#pragma region "AsyncRuntime.when_any"
    private:
        /// Generated by ChatGPT o4-mini-high
        template <std::size_t... Is, typename... Rets>
        Task<std::pair<std::size_t, std::variant<std::monostate, Rets...>>> when_any_impl(std::tuple<Task<Rets>...> tasks, std::index_sequence<Is...>)
        {
            // shared state to record “who finished first”
            std::atomic<bool> triggered{false};
            std::optional<std::pair<std::size_t, std::variant<std::monostate, Rets...>>> winner;

            // spawn one watcher per task—each returns a variant carrying its own result
            auto handles = std::tuple<join_handle<std::variant<std::monostate, Rets...>>...>{
                spawn(
                    // lambda → Task<std::variant<…>>
                    [&, idx = Is](Task<Rets> tsk) -> Task<std::variant<std::monostate, Rets...>>
                    {
                        // co_await the real task
                        auto val = co_await std::move(tsk);

                        // wrap into a variant whose index is (idx+1), reserving 0 for monostate
                        std::variant<std::monostate, Rets...> var{
                            std::in_place_index<idx + 1>,
                            std::move(val)};

                        // record the very first one that races here
                        bool expected = false;
                        if (triggered.compare_exchange_strong(expected, true))
                        {
                            winner = std::pair{idx, var};
                        }

                        co_return var;
                    }(std::get<Is>(tasks)) // immediately invoke the lambda on each task
                    )...};

            // wait until *all* watchers finish (they’re still co_awaiting their tasks)…
            auto all_results = co_await join(std::move(handles));
            // …but by now `winner` holds the index+value of the *first* one that completed
            co_return *winner;
        }

    public:
        /// @brief Executes all the tasks concurrently and returns the first one which finishes and either cancels or discards the other tasks (once first one complete, the other tasks need not complete)
        /// @tparam ...Rets the return arguments of the tasks
        /// @param tasks the tasks to execute
        /// @return the result of the first task to finish
        template <typename... Rets>
        Task<std::pair<std::size_t, std::variant<std::monostate, Rets...>>>
        when_any(std::tuple<Task<Rets>...> tasks)
        {
            return when_any_impl(
                std::move(tasks),
                std::index_sequence_for<Rets...>{});
        }

#pragma endregion

#pragma region "AsyncRuntime.yield"
        /// @brief Yields the task to the caller and lets other tasks in the queue to resume before this one is resumed
        /// @return returns a task which can be awaited
        Task<void> yield();
#pragma endregion

#pragma region "asynchronous services"
        /// @brief Gets the IOService for the runtime.
        /// @return the IO service
        IOService &io_service();

        /// @brief Gets the ExecutorService for the runtime.
        /// @return the executor service
        ExecutorService &executor_service();

        /// @brief Gets the TimerService for the runtime.
        /// @return the timer service
        TimerService &timer_service();
#pragma endregion
    };
}