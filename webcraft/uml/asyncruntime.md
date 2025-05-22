# Async Runtime Package: webcraft::async

## General Awaitable class

This is the general outline of a generic awaitable class. This will be used to template other kinds of awaitables in the framework like IO based awaitables, timer based awaitables, yield based awaitables, thread based awaitables, etc.

```cpp

/// \brief A concept that checks if the type T can be used to be returned from await_suspend
template <typename T>
concept suspend_type = std::same_as<T, std::coroutine_handle<>> ||
                        std::derived_from<T, std::coroutine_handle<>> || std::same_as<T, void> || std::same_as<T, bool>;

/// \brief A concept that checks if the type T is an awaitable type
template <typename T>
concept Awaitable = requires(T t, std::coroutine_handle<> h) {
    { t.await_ready() } -> std::convertible_to<bool>;
    { t.await_suspend(h) } -> suspend_type;
    { t.await_resume() } -> std::same_as<T>;
};

```

## async_runtime

This class provides a runtime for asynchronous functions to be run. The intent of this class was to be built in support of a larger framework which prioritizes both concurrency and parallelism at its core while also going over best practices in software for example choosing Dependency Injection over the Singleton Pattern.

Some of the services provided in `async_runtime` are:
 - running a task: starts the runtime and runs the task on the runtime
 - spawns a task: uses the existing runtime to spawn and run a task concurrently with existing tasks and provides a join handle if completion of spawned task is required
 - joining of tasks: using the join handles of spawned tasks, we can await their completion by doing `co_await runtime.join(jh1)`
 - ensuring that all tasks are completed concurrently: we can have tasks run and complete concurrently instead of sequentially `co_await runtime.all(async_fn1(), async_fn2(), async_fn3())` (potential implementation is spawn all these tasks and await their joining)
 - get the first task which completes: attempt to race tasks with each other and get the result of the first task which completes (good for timeout based IO tasks like `read(bytes, length, waiting_duration)`)
 - yielding current task to the caller queue: allows other queued tasks to resume and suspend before this one is resumed
 - providing other async services such has IO handling (files, sockets, and other IO devices), CPU workers (thread pools for tasks which need to be completed in parallel), and timer services (timeouts, sleeps, and time based jobs)
 - The general outline of the class is shown below

```cpp
/// @brief Singleton-like object that manages and provides a runtime for async operations to occur.
class AsyncRuntime
{
    friend class io::IOService;
    friend class ExecutorService;
    friend class TimerService;

private:
    AsyncRuntime(AsyncRuntimeConfig config);

public:
    AsyncRuntime(const AsyncRuntime &) = delete;
    AsyncRuntime(AsyncRuntime &&) = delete;
    AsyncRuntime &operator=(const AsyncRuntime &) = delete;
    AsyncRuntime &operator=(AsyncRuntime &&) = delete;
    ~AsyncRuntime();

    /// @brief Get the singleton instance of the AsyncRuntime.
    /// @return The singleton instance of the AsyncRuntime.
    static AsyncRuntime &get_instance();

    /// @brief Runs the asynchronous function provided and returns the result.
    /// @tparam T the type of the result of the task.
    /// @tparam ...Args the types of the arguments to the task.
    /// @param fn the function to run.
    /// @return the result of the task.
    template <typename T, typename... Args>
    T run(std::function<Task<T>(Args...)> fn, Args... args);

    /// @brief Runs the task asynchronously and returns the result.
    /// @tparam T the type of the result of the task provided
    /// @param task the task to run.
    /// @return the result of the task.
    template <typename T>
        requires webcraft::not_same_as<T, void>
    T run(Task<T> &&task);

    /// @brief Runs the task asynchronously.
    /// @param task the task to run
    void run(Task<void> &&task);

    /// @brief Spawns a task to run concurrently with the main task and returns a join handle which can be awaited for completion
    /// @param task the task to be spawned
    /// @return the join handle for the task
    join_handle spawn(Task<void> &&task);

    /// @brief Joins all the task handles and passes a task to await their completion
    /// @tparam range the type of the ranges to join
    /// @param handles the range of join handles to join
    /// @return task
    template <std::ranges::range range>
        requires std::same_as<join_handle, std::ranges::range_value_t<range>>
    Task<void> join(range handles);

    /// @brief Executes all the tasks concurrently and returns the result of all tasks in the submitted order
    /// @tparam range the range of the view
    /// @param tasks
    /// @return
    template <std::ranges::range range>
        requires webcraft::not_same_as<Task<void>, std::ranges::range_value_t<range>> && Awaitable<std::ranges::range_value_t<range>>
    auto when_all(range &&tasks) -> Task<std::vector<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>>;

    /// @brief Executes all the tasks concurrently
    /// @tparam range the range of the view
    /// @param tasks the tasks to execute
    /// @return an awaitable
    template <std::ranges::range range>
        requires std::same_as<Task<void>, std::ranges::range_value_t<range>>
    Task<void> when_all(range tasks);

    /// @brief Executes all the tasks concurrently and returns the first one which finishes and either cancels or discards the other tasks (once first one complete, the other tasks need not complete)
    /// @tparam ...Rets the return arguments of the tasks
    /// @param tasks the tasks to execute
    /// @return the result of the first task to finish
    template <std::ranges::range range>
        requires webcraft::not_same_as<Task<void>, std::ranges::range_value_t<range>> && Awaitable<std::ranges::range_value_t<range>>
    auto when_any(range tasks) -> Task<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>;

    /// @brief Yields the task to the caller and lets other tasks in the queue to resume before this one is resumed
    /// @return returns a task which can be awaited
    inline Task<void> yield();

    /// @brief Gets the IOService for the runtime.
    /// @return the IO service
    io::IOService &io_service();

    /// @brief Gets the ExecutorService for the runtime. Allows for tasks to run in parallel
    /// @return the executor service
    ExecutorService &executor_service();

    /// @brief Gets the TimerService for the runtime.
    /// @return the timer service
    TimerService &timer_service();
};
```

## timer_service

This class is a provider for any delay or recurring based actions.

```cpp
class timer_service {
public:
    /// Requires reference to async_runtime to be initialized
    timer_service(async_runtime& runtime);

    /// Sleeps for duration provided asynchronously
    template<typename Rep, typename Period>
    async::task<void> sleep_for(std::chrono::duration<Rep, Period> duration, std::stop_token token);

    /// Sleeps until time point has occured asynchronously
    async::task<void> sleep_until(std::chrono::time_point point, std::stop_token token);

    /// Sets timeout for callback to run and provides a handle in the need to cancel its running
    template<typename Rep, typename Period>
    std::stop_source set_timeout(async_function<task<void>> task, std::chrono::duration<Rep, Period> duration);

    /// Sets timeout for callback to run and provides a handle in the need to cancel its running
    std::stop_source set_timeout(async_function<task<void>> task, std::chrono::time_point point);

    /// Accepts a task supplier to run it periodically every "duration" amount and provides a timer_handle in the case of stopping the "loop"
    template<typename Rep, typename Period>
    std::stop_source set_interval(async_function<void> task_supplier, std::chrono::duration<Rep, Period> duration);
    
    /// Accepts a task supplier to run it periodically every "duration" amount and provides a timer_handle in the case of stopping the "loop"
    std::stop_source set_interval(async_function<void> task_supplier, std::chrono::time_point point);

};

```

## executor_service

This class presents an interface for tasks to be run in parallel with each other leveraging the CPU cores by using thread pools. Also note that any timer based task or io based task if run on the pool will jump off the pool onto the main thread so if you need to do CPU bound tasks after you will need to reschedule onto the pool.

```cpp

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

    ExecutorService(AsyncRuntime &runtime, ExecutorServiceParams &params);

public:
    ~ExecutorService();
    ExecutorService(const ExecutorService &) = delete;
    ExecutorService(ExecutorService &&) = delete;
    ExecutorService &operator=(const ExecutorService &) = delete;
    ExecutorService &operator=(ExecutorService &&) = delete;

    /// Schedules the current coroutine onto the thread pool
    Task<void> schedule(SchedulingPriority priority = SchedulingPriority::LOW);

    /// Schedules an async function to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
    join_handle schedule(Task<void> task, SchedulingPriority priority = SchedulingPriority::LOW);

    /// @brief Runs the tasks in parallel
    /// @param tasks the tasks to run in parallel
    /// @return an awaitable
    template <std::ranges::range range>
        requires std::same_as<Task<void>, std::ranges::range_value_t<range>>
    inline Task<void> runParallel(range tasks);

    /// @brief Runs the tasks in parallel and return the results in the order provided
    /// @param tasks the tasks to run in parallel
    /// @return the results of the tasks in the order provided
    template <std::ranges::range range>
        requires webcraft::not_same_as<Task<void>, std::ranges::range_value_t<range>> && Awaitable<std::ranges::range_value_t<range>>
    auto runParallel(range tasks) -> Task<std::vector<::async::awaitable_resume_t<std::ranges::range_value_t<range>>>>;

};
```

## io_service

This class represents the provider for any IO bound tasks and services.. anything regarding files, sockets, and other IO devices.

```cpp
class io_service {
public:
    io_service(async_runtime& runtime);

    /// Gets the file from the filesystem path
    std::shared_ptr<File> get_file(std::filesystem::path path);

    /// Creates a client socket
    std::shared_ptr<Socket> create_tcp_connect_socket();

    /// Creates server socket ... performs bind & listen
    std::shared_ptr<ServerSocket> create_tcp_server_socket(socket_address addr);
};
```

### file

```cpp
class file {
private:
    // private constructor... cuz we don't want anyone to create unmanaged files
public:
    
    size_t size();

    bool is_file();

    bool is_directory();

    bool exists();

    bool create_file();

    bool create_if_not_exists();

    bool create_folder();

    std::string name();

    std::string extension();

    std::filesystem::path path();

    std::vector<std::filesystem::path> list_file_paths();

    std::vector<File> list_files();

    async_read_stream open_read_stream();

    async_write_stream open_write_stream();

    async_write_stream open_append_stream();

    // ... more other methods as I go
};
```

### socket

```cpp
class socket {
private:
    // private constructor... cuz we don't want anyone to create unmanaged sockets
public:

    async::task<void> connect(socket_address address);

    socket_address get_address();

    async_stream get_stream();

    void close();

    // ... more other methods as I go
};
```

```cpp
class server_socket {
private:
    // private constructor... cuz we don't want anyone to create unmanaged sockets
public:

    socket_address get_address();

    void listen();

    async::task<std::shared_ptr<Socket>> accept();

    void close();

    // ... more other methods as I go
};
```