#include <webcraft/async/config.hpp>
#include <webcraft/async/runtime.hpp>

static webcraft::async::async_runtime_config config = {
    .max_worker_threads = 2 * std::thread::hardware_concurrency(),
    .min_worker_threads = std::thread::hardware_concurrency(),
    .idle_timeout = 30s,
    .worker_strategy = webcraft::async::worker_strategy_type::PRIORITY};

/// Macro to generate helper setter methods for the cofig
#define set_async_runtime_config_field(type, field)           \
    void webcraft::async::runtime_config::set_##field(type t) \
    {                                                         \
        config.field = t;                                   \
    }

set_async_runtime_config_field(size_t, max_worker_threads);
set_async_runtime_config_field(size_t, min_worker_threads);
set_async_runtime_config_field(std::chrono::milliseconds, idle_timeout);
set_async_runtime_config_field(webcraft::async::worker_strategy_type, worker_strategy);

webcraft::async::async_runtime &webcraft::async::async_runtime::get_instance()
{
    // lazily initialize the instance (allow for config setup before you get the first instance)
    static async_runtime runtime(config);
    return runtime;
}

// TODO: create the definitions for the async runtime: constructor, destructor, queue_task_resumption, & run
webcraft::async::async_runtime::async_runtime(async_runtime_config &config)
{
}

webcraft::async::async_runtime::~async_runtime()
{
}

void webcraft::async::async_runtime::queue_task_resumption(std::coroutine_handle<> h)
{
}

void webcraft::async::async_runtime::run(webcraft::async::task<void> &&t)
{
}