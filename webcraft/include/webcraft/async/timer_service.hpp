#pragma once

#include <chrono>
#include <concepts>
#include <webcraft/async/runtime.hpp>

namespace webcraft::async
{
    class timer_service
    {
    private:
        friend class async_runtime;
        async_runtime &runtime;

#pragma region "constructors and destructors"
    protected:
        timer_service(async_runtime &runtime);

    public:
        ~timer_service();
        timer_service(const timer_service &) = delete;
        timer_service(timer_service &&) = delete;
        timer_service &operator=(const timer_service &) = delete;
        timer_service &operator=(timer_service &&) = delete;
#pragma endregion

#pragma region "sleep"

        /// @brief Sleeps for duration provided asynchronously
        /// @param duration duration to sleep for
        /// @param token stop token to cancel the sleep operation
        /// @return an awaitable
        // TODO: implement this
        task<void> sleep_for(std::chrono::steady_clock::duration duration, std::stop_token token = {});

        /// @brief Sleeps for duration provided asynchronously
        /// @tparam Rep the representation type of the duration
        /// @tparam Period the period of the duration
        /// @param duration the duration to sleep for
        /// @param token the stop token to cancel the sleep operation
        /// @return an awaitable
        template <typename Rep, typename Period>
            requires webcraft::not_true<std::same_as<std::chrono::duration<Rep, Period>, std::chrono::steady_clock::duration>>
        inline task<void> sleep_for(std::chrono::duration<Rep, Period> duration, std::stop_token token = {})
        {
            return sleep_for(std::chrono::duration_cast<std::chrono::steady_clock::duration>(duration), token);
        }

        /// @brief Sleeps until time point has occured asynchronously
        /// @tparam Rep the representation type of the duration
        /// @tparam Period the period of the duration
        /// @param point the time point to sleep until
        /// @param token the stop token to cancel the sleep operation
        /// @return an awaitable
        template <typename Rep, typename Period>
        inline task<void> sleep_until(std::chrono::time_point<Rep, Period> point, std::stop_token token = {})
        {
            auto duration = point - std::chrono::steady_clock::now();
            return sleep_for(std::chrono::duration_cast<std::chrono::steady_clock::duration>(duration), token);
        }

#pragma endregion

#pragma region "set timeout"

        using cancellable_async_function = std::function<task<void>(std::stop_token)>;

        /// @brief Sets timeout for callback to run and provides a stop source in the need to cancel its running
        /// @param task_supplier the asynchronous function to be executed
        /// @param duration the duration for the task to be executed
        /// @return the stop token which can be used to cancel the task
        template <typename Rep, typename Period>
        std::stop_source set_timeout(cancellable_async_function task_supplier, std::chrono::duration<Rep, Period> duration)
        {
            std::stop_source src;
            auto token = src.get_token();

            auto spawn_fn = [this, task_supplier = std::move(task_supplier), duration, token]() -> task<void>
            {
                co_await sleep_for(duration, token);
                if (token.stop_requested())
                    co_return;
                co_await task_supplier(token);
            };

            runtime.spawn(spawn_fn());

            return src;
        }

        /// @brief Set timeout for callback to run and provides a stop source in the need to cancel its running
        /// @param task_supplier the asynchronous function to be executed
        /// @param point the point in time when the task should be executed
        /// @return the stop token which can be used to cancel the task
        template <typename Rep, typename Period>
        std::stop_source set_timeout(cancellable_async_function task_supplier, std::chrono::time_point<Rep, Period> point)
        {
            std::stop_source src;
            auto token = src.get_token();

            auto spawn_fn = [this, task_supplier = std::move(task_supplier), point, token]() -> task<void>
            {
                co_await sleep_until(point, token);
                if (token.stop_requested())
                    co_return;
                co_await task_supplier(token);
            };

            runtime.spawn(spawn_fn());

            return src;
        }

#pragma endregion

#pragma region "set interval"

        /// @brief Accepts a task supplier to run it periodically every "duration" amount and provides a stop source in the case of stopping the "loop"
        /// @param task_supplier the asynchronous function to be executed
        /// @param duration the duration for the task to be executed
        /// @return the stop token which can be used to cancel the task
        template <typename Rep, typename Period>
        std::stop_source set_interval(cancellable_async_function task_supplier, std::chrono::duration<Rep, Period> duration)
        {
            std::stop_source src;
            auto token = src.get_token();

            auto spawn_fn = [this, task_supplier = std::move(task_supplier), duration, token]() -> task<void>
            {
                while (!token.stop_requested())
                {
                    co_await sleep_for(duration, token);
                    if (token.stop_requested())
                        co_return;

                    co_await task_supplier(token);
                }
            };

            runtime.spawn(spawn_fn());

            return src;
        }

#pragma endregion
    };
}