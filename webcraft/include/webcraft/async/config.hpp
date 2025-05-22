#pragma once
#include <thread>

namespace webcraft::async
{
    class AsyncRuntime;

    enum class WorkerStrategyType
    {
        /// @brief Implements a work stealing strategy for the executor service
        WORK_STEALING,
        /// @brief Implements a cached strategy for the executor service
        CACHED,
        /// @brief Implements a hybrid of the work stealing and cached strategies for the executor service
        HYBRID,
        /// @brief Implements a priority based strategy for the executor service (low priority tasks are handled by work stealing pools and high priority tasks are handled by cached pools)
        PRIORITY
    };

    /// @brief Configuration class for the AsyncRuntime.
    /// This class allows the user to configure the AsyncRuntime before it is created.
    struct AsyncRuntimeConfig
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
        size_t max_worker_threads = 2 * std::thread::hardware_concurrency();
        size_t min_worker_threads = std::thread::hardware_concurrency();
        size_t idle_timeout = 30000;
        WorkerStrategyType worker_strategy = WorkerStrategyType::PRIORITY;
    };
}