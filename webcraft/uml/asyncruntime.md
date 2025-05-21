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
class async_runtime {
private:
    friend class io_service;
    friend class worker_service;
    friend class timer_service;

public:
    template<typename T, typename... ARGS>
    using async_function = std::function<async::task<T>(...ARGS)>;

    /// Runs the task synchronously
    template<typename T>
    T run(async_function<T, async_runtime&> entry_point);

    /// Spawns a task to run concurrently with the main task and returns a join handle which can be awaited for completion
    join_handle spawn(async::task<void> task);

    /// Joins all the task handles and passes a task to await their completion
    async::task<void> join(join_handle... handles)

    /// Executes all the tasks concurrently and returns the result of all tasks in the submitted
    template<typename... RET>
    async::task<std::tuple<RET...>> all(async::task<RET>... tasks);

    /// Executes all the tasks concurrently and returns the first one which finishes and either cancels or discards the other tasks (once first one complete, the other tasks need not complete)
    template<typename... RET>
    async::task<std::variant<RET...>> any(async::task<RET>... tasks);

    /// Yields the task to the caller and lets other tasks in the queue to resume before this one is resumed
    async::task<void> yield();

    /// Returns the io_service associated with the runtime
    io_service io_service();

    /// Returns the worker_service associated with the runtime
    worker_service worker_service();

    /// Returns the timer_service associated with the runtime
    timer_service timer_service();
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

## worker_service

This class presents an interface for tasks to be run in parallel with each other leveraging the CPU cores by using thread pools. Also note that any timer based task or io based task if run on the pool will jump off the pool onto the main thread so if you need to do CPU bound tasks after you will need to reschedule onto the pool.

```cpp
class worker_service {
public:
    worker_service(async_runtime& runtime, size_t workers);

    /// Schedules the current coroutine onto the thread pool
    async::task<void> schedule();

    /// Schedules an async function to be run on the thread pool asynchronously and provides a join handle which can be awaited to await completion
    join_handle schedule(async_function<void> task_supplier);
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