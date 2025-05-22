#pragma once

#include <async/awaitable_resume_t.h>
#include <webcraft/concepts.hpp>
#include <webcraft/async/awaitable.hpp>
#include <webcraft/async/join_handle.hpp>
#include <webcraft/async/runtime.hpp>
#include <ranges>

namespace webcraft::async
{
    /// @brief The priority of the task to be scheduled
    enum class SchedulingPriority
    {
        LOW,
        HIGH
    };

    /// @brief the parameters to initialize the executor service
    struct ExecutorServiceParams
    {
        int minWorkers;
        int maxWorkers;
        int idleTimeout;
        WorkerStrategyType strategy;
    };

    /// @brief the executor service strategy
    class Executor
    {
    public:
        /// Schedules the current coroutine onto the thread pool
        virtual Task<void> schedule(SchedulingPriority priority = SchedulingPriority::LOW) = 0;

        /// Schedules an async function to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
        virtual join_handle schedule(Task<void> task, SchedulingPriority priority = SchedulingPriority::LOW) = 0;
    };

    /// @brief A class that represents an executor service that can be used to run tasks asynchronously.
    class ExecutorService
    {
    private:
        friend class AsyncRuntime;
        AsyncRuntime &runtime;
        std::unique_ptr<Executor> strategy; // strategy for the executor service
                                            // TODO: implement the strategies for this in the cpp file

#pragma region "constructors and destructors"
        // TODO: implement the constructor
        ExecutorService(AsyncRuntime &runtime, ExecutorServiceParams &params);

    public:
        ~ExecutorService();
        ExecutorService(const ExecutorService &) = delete;
        ExecutorService(ExecutorService &&) = delete;
        ExecutorService &operator=(const ExecutorService &) = delete;
        ExecutorService &operator=(ExecutorService &&) = delete;
#pragma endregion

#pragma region "scheduling"

        /// @brief Schedules the current coroutine onto the executor
        /// @param priority the priority the coroutine should run at
        /// @return an awaitable
        inline Task<void> schedule(SchedulingPriority priority = SchedulingPriority::LOW)
        {
            return strategy->schedule(priority);
        }

        /// @brief Schedules the current coroutine onto the executor with a low priority
        /// @return an awaitable
        inline Task<void> schedule_low()
        {
            return schedule(SchedulingPriority::LOW);
        }

        /// @brief Schedules the current coroutine onto the executor with a high priority
        /// @return an awaitable
        inline Task<void> schedule_high()
        {
            return schedule(SchedulingPriority::HIGH);
        }

        /// @brief Schedules an async task to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
        /// @param task schedules the task to run on the executor
        /// @param priority the priority the task should run with
        /// @return a join handle to await completion
        inline join_handle schedule(Task<void> task, SchedulingPriority priority = SchedulingPriority::LOW)
        {
            return strategy->schedule(task, priority);
        }

        /// @brief Schedules an async task with a low priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @param task schedules the task to run on the executor
        /// @return a join handle to await completion
        inline join_handle schedule_low(Task<void> task)
        {
            return strategy->schedule(task, SchedulingPriority::LOW);
        }

        /// @brief Schedules an async task with a high priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @param task schedules the task to run on the executor
        /// @return a join handle to await completion
        inline join_handle schedule_high(Task<void> task)
        {
            return strategy->schedule(task, SchedulingPriority::HIGH);
        }

        /// @brief Schedules an async function with a low priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @tparam T the return type
        /// @tparam ...Args the types of arguments for the function
        /// @param async_fn the asynchronous function
        /// @param ...args the arguments for the asynchronous function
        /// @return a join handle
        template <typename T, typename... Args>
        join_handle schedule_low(std::function<Task<T>(Args...)> async_fn, Args... args)
        {
            return schedule_low(async_fn(args...));
        }

        /// @brief Schedules an async function with a high priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @tparam T the return type
        /// @tparam ...Args the types of arguments for the function
        /// @param async_fn the asynchronous function
        /// @param ...args the arguments for the asynchronous function
        /// @return a join handle
        template <typename T, typename... Args>
        join_handle schedule_high(std::function<Task<T>(Args...)> async_fn, Args... args)
        {
            return schedule_high(async_fn(args...));
        }

        /// @brief Schedules a function with a low priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @tparam T the return type
        /// @tparam ...Args the types of the arguments for the function
        /// @param fn the function to be executed
        /// @param ...args the arguments for the functions execution
        /// @return the join handle
        template <typename T, typename... Args>
            requires webcraft::not_true<Awaitable<T>>
        join_handle schedule_low(std::function<T(Args...)> fn, Args... args)
        {
            auto async_fn = [](Args... args) -> Task<T>
            {
                co_return fn(args...);
            };

            return schedule_low(async_fn(args...));
        }

        /// @brief Schedules a function with a high priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @tparam T the return type
        /// @tparam ...Args the types of the arguments for the function
        /// @param fn the function to be executed
        /// @param ...args the arguments for the functions execution
        /// @return the join handle
        template <typename T, typename... Args>
        join_handle schedule_high(std::function<T(Args...)> fn, Args... args)
        {
            auto async_fn = [](Args... args) -> Task<T>
            {
                co_return fn(args...);
            };

            return schedule_high(async_fn(args...));
        }

#pragma endregion

#pragma region "parallel processing"
        /// @brief Runs the tasks in parallel
        /// @param tasks the tasks to run in parallel
        /// @return an awaitable
        template <std::ranges::range range>
            requires std::same_as<Task<void>, std::ranges::range_value_t<range>>
        inline Task<void> runParallel(range tasks)
        {
            // schedule and join the tasks
            co_await runtime.join(tasks | std::ranges::transform([&](Task<void> task)
                                                                 { return schedule(task); }));
        }

        template <std::ranges::range range>
            requires webcraft::not_same_as<Task<void>, std::ranges::range_value_t<range>> && Awaitable<std::ranges::range_value_t<range>>
        auto runParallel(range tasks) -> Task<std::vector<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>>
        {
            using T = ::async::awaitable_resume_t<std::ranges::range_value_t<range>>;

            // schedule each task and immediately collect their handles
            // join the handles and return the result
            std::vector<std::optional<T>> vec;

            // Assign each task a return destination, schedule them all and join them
            co_await runtime.join(tasks | std::ranges::transform(
                                              [&](Task<T> task)
                                              {
                                                  vec.push_back({std::nullopt});
                                                  size_t index = vec.size() - 1;

                                                  auto fn = [vec, index](Task<T> task) -> Task<T>
                                                  {
                                                      vec[index] = co_await task;
                                                  };

                                                  // schedule the tasks
                                                  return schedule(fn(task));
                                              }));

            // convert all of the values from optionals and return vector of them
            co_return vec | std::transform([](std::optional<T> opt)
                                           { return opt.value(); }) |
                std::ranges::to<std::vector>();
        }
#pragma endregion
    };
}
