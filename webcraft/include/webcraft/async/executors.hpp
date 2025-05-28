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
    enum class scheduling_priority
    {
        LOW,
        HIGH
    };

    /// @brief the parameters to initialize the executor service
    struct executor_service_params
    {
        size_t minWorkers;
        size_t maxWorkers;
        std::chrono::milliseconds idleTimeout;
        worker_strategy_type strategy;
    };

    /// @brief the executor service strategy
    class executor
    {
    public:
        /// Schedules the current coroutine onto the thread pool
        virtual task<void> schedule(scheduling_priority priority = scheduling_priority::LOW) = 0;

        /// Schedules an async function to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
        virtual join_handle schedule(task<void> task, scheduling_priority priority = scheduling_priority::LOW) = 0;
    };

    /// @brief A class that represents an executor service that can be used to run tasks asynchronously.
    class executor_service
    {
    private:
        friend class async_runtime;
        async_runtime &runtime;
        std::unique_ptr<executor> strategy; // strategy for the executor service
                                            // TODO: implement the strategies for this in the cpp file

#pragma region "constructors and destructors"
    protected:
        // TODO: implement the constructor
        executor_service(async_runtime &runtime, executor_service_params &params);

    public:
        ~executor_service();
        executor_service(const executor_service &) = delete;
        executor_service(executor_service &&) = delete;
        executor_service &operator=(const executor_service &) = delete;
        executor_service &operator=(executor_service &&) = delete;
#pragma endregion

#pragma region "scheduling"

        /// @brief Schedules the current coroutine onto the executor
        /// @param priority the priority the coroutine should run at
        /// @return an awaitable
        inline task<void> schedule(scheduling_priority priority = scheduling_priority::LOW)
        {
            return strategy->schedule(priority);
        }

        /// @brief Schedules the current coroutine onto the executor with a low priority
        /// @return an awaitable
        inline task<void> schedule_low()
        {
            return schedule(scheduling_priority::LOW);
        }

        /// @brief Schedules the current coroutine onto the executor with a high priority
        /// @return an awaitable
        inline task<void> schedule_high()
        {
            return schedule(scheduling_priority::HIGH);
        }

        /// @brief Schedules an async task to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
        /// @param task schedules the task to run on the executor
        /// @param priority the priority the task should run with
        /// @return a join handle to await completion
        inline join_handle schedule(task<void> task, scheduling_priority priority = scheduling_priority::LOW)
        {
            return strategy->schedule(task, priority);
        }

        /// @brief Schedules an async task with a low priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @param task schedules the task to run on the executor
        /// @return a join handle to await completion
        inline join_handle schedule_low(task<void> task)
        {
            return strategy->schedule(task, scheduling_priority::LOW);
        }

        /// @brief Schedules an async task with a high priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @param task schedules the task to run on the executor
        /// @return a join handle to await completion
        inline join_handle schedule_high(task<void> task)
        {
            return strategy->schedule(task, scheduling_priority::HIGH);
        }

        /// @brief Schedules an async function with a low priority to be run on the executor asynchronously and provides a join handle which can be awaited to await completion
        /// @tparam T the return type
        /// @tparam ...Args the types of arguments for the function
        /// @param async_fn the asynchronous function
        /// @param ...args the arguments for the asynchronous function
        /// @return a join handle
        template <typename T, typename... Args>
        join_handle schedule_low(std::function<task<T>(Args...)> async_fn, Args... args)
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
        join_handle schedule_high(std::function<task<T>(Args...)> async_fn, Args... args)
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
            requires webcraft::not_true<awaitable<T>>
        join_handle schedule_low(std::function<T(Args...)> fn, Args... args)
        {
            auto async_fn = [fn = std::move(fn)](Args... args) -> task<T>
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
            auto async_fn = [fn = std::move(fn)](Args... args) -> task<T>
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
            requires std::same_as<task<void>, std::ranges::range_value_t<range>>
        inline task<void> runParallel(range tasks)
        {
            // schedule and join the tasks
            co_await runtime.join(tasks | std::ranges::transform([&](task<void> task)
                                                                 { return schedule(task); }));
        }

        template <std::ranges::range range>
            requires webcraft::not_same_as<task<void>, std::ranges::range_value_t<range>> && awaitable<std::ranges::range_value_t<range>>
        auto runParallel(range tasks) -> task<std::vector<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>>
        {
            using T = ::async::awaitable_resume_t<std::ranges::range_value_t<range>>;

            // schedule each task and immediately collect their handles
            // join the handles and return the result
            std::vector<std::optional<T>> vec;

            // Assign each task a return destination, schedule them all and join them
            co_await runtime.join(tasks | std::ranges::transform(
                                              [&](task<T> t)
                                              {
                                                  vec.push_back({std::nullopt});
                                                  size_t index = vec.size() - 1;

                                                  auto fn = [vec, index](task<T> t) -> task<T>
                                                  {
                                                      vec[index] = co_await t;
                                                  };

                                                  // schedule the tasks
                                                  return schedule(fn(t));
                                              }));

            auto pipe = vec | std::views::transform([](std::optional<T> opt)
                                                    { return opt.value(); });
            std::vector<T> out;
            out.reserve(std::ranges::distance(pipe));
            std::ranges::copy(pipe, std::back_inserter(out));
            co_return out;
        }
#pragma endregion
    };
}
