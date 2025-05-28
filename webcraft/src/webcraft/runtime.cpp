#include <webcraft/async/config.hpp>
#include <webcraft/async/runtime.hpp>
#include <webcraft/async/executors.hpp>

static webcraft::async::async_runtime_config config = {
    .max_worker_threads = 2 * std::thread::hardware_concurrency(),
    .min_worker_threads = std::thread::hardware_concurrency(),
    .idle_timeout = 30s,
    .worker_strategy = webcraft::async::worker_strategy_type::PRIORITY};

/// Macro to generate helper setter methods for the cofig
#define set_async_runtime_config_field(type, field)           \
    void webcraft::async::runtime_config::set_##field(type t) \
    {                                                         \
        config.field = t;                                     \
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
    webcraft::async::executor_service_params params = {
        .minWorkers = config.min_worker_threads,
        .maxWorkers = config.max_worker_threads,
        .idleTimeout = config.idle_timeout,
        .strategy = config.worker_strategy};

    this->executor_svc = std::make_unique<webcraft::async::executor_service>(*this, params);
    this->timer_svc = std::make_unique<webcraft::async::timer_service>(*this);
}

webcraft::async::async_runtime::~async_runtime()
{
}

void webcraft::async::async_runtime::queue_task_resumption(std::coroutine_handle<> h)
{
#ifdef _WIN32
// PostCompletionStatus() OVERLAPPED
#elif defined(__linux__)
// io_uring_submit_sqe
#elif defined(__APPLE__)

#else

#endif
}

void webcraft::async::async_runtime::run(webcraft::async::task<void> &&t)
{

#ifdef _WIN32
// PostCompletionStatus() OVERLAPPED
#elif defined(__linux__)
// io_uring_submit_sqe
#elif defined(__APPLE__)

#else

#endif
}

void webcraft::async::unsafe::initialize_runtime_handle(webcraft::async::unsafe::native_runtime_handle &handle)
{
#ifdef _WIN32
    // iocp https://stackoverflow.com/questions/53651391/which-handle-to-pass-to-createiocompletionport-if-using-iocp-for-basic-signalin
    handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0); // Create an IOCP handle
    if (handle == nullptr)
    {
        throw std::runtime_error("Failed to create IOCP handle: " + std::to_string(GetLastError()));
    }
#elif defined(__linux__)
    // io_uring https://pabloariasal.github.io/2022/11/12/couring-1/
    if (io_uring_queue_init(IO_URING_QUEUE_SIZE, &handle, 0) < 0) // Initialize the io_uring queue with a size of 256
    {
        throw std::runtime_error("Failed to initialize io_uring queue");
    }
#elif defined(__APPLE__)
    // kqueue https://repo.or.cz/eleutheria.git/blob/master:/kqueue/kqclient.c
    handle = kqueue(); // Create a kqueue handle
    if (handle == -1)
    {
        throw std::runtime_error("Failed to create kqueue handle: " + std::to_string(errno));
    }
#else
#endif
}

void webcraft::async::unsafe::destroy_runtime_handle(webcraft::async::unsafe::native_runtime_handle &handle)
{
#ifdef _WIN32
    CloseHandle(handle); // Close the IOCP handle
#elif defined(__linux__)
    io_uring_queue_exit(&handle); // exit the io_uring queue
#elif defined(__APPLE__)
    close(handle); // Close the kqueue handle
#else
#endif
}