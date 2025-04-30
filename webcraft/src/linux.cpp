#ifdef __linux__

#include "webcraft/webcraft.hpp"
#include <fmt/core.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <liburing.h>
#include <vector>
#include <functional>

class IOUring
{
public:
    explicit IOUring(size_t queue_size)
    {
        if (auto s = io_uring_queue_init(queue_size, &ring_, 0); s < 0)
        {
            throw std::runtime_error("error initializing io_uring: " + std::to_string(s));
        }
    }

    IOUring(const IOUring &) = delete;
    IOUring &operator=(const IOUring &) = delete;
    IOUring(IOUring &&) = delete;
    IOUring &operator=(IOUring &&) = delete;

    ~IOUring() { io_uring_queue_exit(&ring_); }

    struct io_uring *get()
    {
        return &ring_;
    }

private:
    struct io_uring ring_;
};

struct IOEvent
{
    std::function<void(int)> callback;
};

struct IOAwaitable
{
private:
    struct io_uring_sqe *sqe;
    int result;

public:
    IOAwaitable(struct io_uring_sqe *sqe)
        : sqe(sqe) {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> handle)
    {
        auto event = new IOEvent{[&](int res)
                                 {
                                     result = res;
                                     handle.resume();
                                 }};
        io_uring_sqe_set_data(sqe, event);
    }

    int await_resume()
    {
        return result;
    }
};

off_t get_file_size(int fd)
{
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        return -1;
    }
    return st.st_size;
}

class ReadOnlyFile
{
public:
    ReadOnlyFile(const std::string &file_path) : path_{file_path}
    {
        fd_ = open(file_path.c_str(), O_RDONLY);
        if (fd_ < 0)
        {
            throw std::runtime_error("Fail to open file");
        }
        size_ = get_file_size(fd_);
        if (size_ < 0)
        {
            throw std::runtime_error("Fail to get size of file");
        }
    }

    ReadOnlyFile(ReadOnlyFile &&other)
        : path_{std::exchange(other.path_, {})},
          fd_{std::exchange(other.fd_, -1)},
          size_{other.size()} {}

    ~ReadOnlyFile()
    {
        if (fd_)
        {
            close(fd_);
        }
    }

    int fd() const { return fd_; }
    off_t size() const { return get_file_size(fd_); }
    const std::string &path() const { return path_; }

private:
    std::string path_;
    int fd_;
    off_t size_;
};

async::task<std::string> read_file_async(IOUring &io_uring, const std::string &filename)
{
    // Simulate an asynchronous file read operation
    ReadOnlyFile file(filename);
    auto fd = file.fd();
    auto size = file.size();

    std::vector<char> buffer(size);
    struct io_uring_sqe *sqe = io_uring_get_sqe(io_uring.get());
    if (!sqe)
    {
        throw std::runtime_error("Failed to get SQE");
    }

    io_uring_prep_read(sqe, file.fd(), buffer.data(), buffer.size(), 0);

    co_await IOAwaitable(sqe);

    co_return std::string(buffer.data(), buffer.size());
}

async::task<void> read_entries(IOUring &io_uring)
{
    auto str = co_await read_file_async(io_uring, "./helloworld.txt");

    fmt::println("Read from file: {}\n", str);
    co_return;
}

struct fire_and_forget
{
    struct promise_type
    {
        void return_void() {}
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        fire_and_forget get_return_object() { return {}; }
        void unhandled_exception() { std::terminate(); }
    };
};

fire_and_forget run_task_on_io(async::task<void> task, async::event_signal &signal)
{
    co_await task;
    signal.set();
}

void run_app()
{
    fmt::println("Running on Linux");

    // Create the IOUring instance with a queue size of 256
    IOUring io_uring(256);

    // Call the coroutine to read entries and eagerly run it - the rest of the coroutine will be completed in the IO loop
    async::event_signal signal;

    run_task_on_io(read_entries(io_uring), signal);

    // Create a loop to read entries from the completion queue

    while (!signal.is_set())
    {
        io_uring_submit_and_wait(io_uring.get(), 1);
        struct io_uring_cqe *cqe;

        unsigned head;
        unsigned processed = 0;
        io_uring_for_each_cqe(io_uring.get(), head, cqe)
        {
            auto event = static_cast<IOEvent *>(io_uring_cqe_get_data(cqe));

            if (event)
            {
                // Process the event
                event->callback(cqe->res);

                delete event;
                processed++;
            }
        }

        io_uring_cq_advance(io_uring.get(), processed);
    }
}

#endif