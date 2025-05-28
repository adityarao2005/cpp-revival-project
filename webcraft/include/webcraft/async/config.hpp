#pragma once
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

namespace webcraft::async
{
    class async_runtime;

    enum class worker_strategy_type
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

    /// @brief Configuration class for the async_runtime.
    /// This class allows the user to configure the async_runtime before it is created.
    struct async_runtime_config
    {
        // TODO: add more configuration options
        size_t max_worker_threads = 2 * std::thread::hardware_concurrency();
        size_t min_worker_threads = std::thread::hardware_concurrency();
        std::chrono::milliseconds idle_timeout = 30s;
        worker_strategy_type worker_strategy = worker_strategy_type::PRIORITY;
    };

    namespace runtime_config
    {

        /// @brief Sets the maximum number of worker threads for the async_runtime.
        void set_max_worker_threads(size_t max_threads);
        /// @brief Sets the minimum number of worker threads for the async_runtime.
        void set_min_worker_threads(size_t min_threads);
        /// @brief Sets the idle timeout for the async_runtime.
        void set_idle_timeout(std::chrono::milliseconds timeout);
        /// @brief Sets the worker strategy for the async_runtime.
        void set_worker_strategy(worker_strategy_type strategy);
    }
}