#pragma once

#include <async/awaitable_resume_t.h>
#include <webcraft/concepts.hpp>
#include <webcraft/async/awaitable.hpp>
#include <webcraft/async/join_handle.hpp>
#include <webcraft/async/runtime.hpp>
#include <ranges>

namespace webcraft::async
{
    enum class SchedulingPriority
    {
        LOW,
        HIGH
    };

    struct ExecutorServiceParams
    {
        int minWorkers;
        int maxWorkers;
        int idleTimeout;
        WorkerStrategyType strategy;
    };

    class ExecutorServiceStrategy;

    /// @brief A class that represents an executor service that can be used to run tasks asynchronously.
    class ExecutorService
    {
    private:
        friend class AsyncRuntime;
        AsyncRuntime &runtime;
        std::unique_ptr<ExecutorServiceStrategy> strategy; // strategy for the executor service
        // TODO: implement the strategies for this in the cpp file

#pragma region "constructors and destructors"
        ExecutorService(AsyncRuntime &runtime, ExecutorServiceParams &params);

    public:
        ~ExecutorService();
        ExecutorService(const ExecutorService &) = delete;
        ExecutorService(ExecutorService &&) = delete;
        ExecutorService &operator=(const ExecutorService &) = delete;
        ExecutorService &operator=(ExecutorService &&) = delete;
#pragma endregion

#pragma region "scheduling"

        /// Schedules the current coroutine onto the thread pool
        Task<void> schedule(SchedulingPriority priority = SchedulingPriority::LOW);

        /// Schedules an async function to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
        join_handle schedule(Task<void> task, SchedulingPriority priority = SchedulingPriority::LOW);

        /// @brief Runs the tasks in parallel
        /// @param tasks the tasks to run in parallel
        /// @return an awaitable
        template <std::ranges::range range>
            requires std::same_as<Task<void>, std::ranges::range_value_t<range>>
        inline Task<void> runParallel(range tasks)
        {
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

            co_await runtime.join(tasks | std::ranges::transform(
                                              [&](Task<T> task)
                                              {
                                                  vec.push_back({std::nullopt});
                                                  size_t index = vec.size() - 1;

                                                  auto fn = [vec, index](Task<T> task) -> Task<T>
                                                  {
                                                      vec[index] = co_await task;
                                                  };

                                                  return schedule(fn(task));
                                              }));

            co_return vec | std::transform([](std::optional<T> opt)
                                           { return opt.value(); }) |
                std::ranges::to<std::vector>();
        }
#pragma endregion
    };
}
