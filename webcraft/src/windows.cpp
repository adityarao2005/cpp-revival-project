#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#include <fmt/core.h>
#include "webcraft/webcraft.hpp"

/*
https://github.com/lewissbaker/cppcoro/blob/master/lib/io_service.cpp#L308
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/io_service.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/readable_file.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/file_read_operation.hpp
https://github.com/lewissbaker/cppcoro/blob/master/lib/file_read_operation.cpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/detail/win32_overlapped_operation.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/detail/win32.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/file.hpp
https://github.com/lewissbaker/cppcoro/tree/master?tab=readme-ov-file#io_service-and-io_work_scope
https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-overlapped

essentially to run it with windows what you need to do is:

create an IoCompletionPort (this will act as your queue of events)

then create your io device and some threads if you want

then create a thread pool and assign the threads to the IoCompletionPort

then create your async task and assign it to the IoCompletionPort
 - Just call teh ReadFile, WriteFile, etc. functions with the IoCompletionPort as the first argument

then wait for the events to be processed in the thread pool
 - Just call the GetQueuedCompletionStatus function with the IoCompletionPort as the first argument

pseudocode:

initWinSock()
handle = CreateIoCompletionPort()
task = RunTask() // read from file and print from console
coroutine = task.coro

coroutine.resume() // start the task wait for it to make the "first" async call (ReadFile, WriteFile, etc.) then have it suspend

Overlapped* ov;
while GetQueuedCompletionStatus(handle, ov, ...) {

    callback = ((Event) ov)->callback
    callback(); // will resume coroutine here
}



*/

class smart_handle
{
private:
    HANDLE handle_;

public:
    smart_handle(HANDLE handle) : handle_(handle) {}
    ~smart_handle()
    {
        if (handle_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
        }
    }

    smart_handle(const smart_handle &) = delete;            // Disable copy constructor
    smart_handle &operator=(const smart_handle &) = delete; // Disable copy assignment operator

    smart_handle(smart_handle &&other) : handle_(other.handle_)
    {
        other.handle_ = INVALID_HANDLE_VALUE; // Transfer ownership
    }
    smart_handle &operator=(smart_handle &&other)
    {
        if (this != &other)
        {
            if (handle_ != INVALID_HANDLE_VALUE)
            {
                CloseHandle(handle_);
            }
            handle_ = other.handle_;
            other.handle_ = INVALID_HANDLE_VALUE; // Transfer ownership
        }
        return *this;
    }

    HANDLE get() const { return handle_; }
    operator HANDLE() const { return handle_; }
    bool is_valid() const { return handle_ != INVALID_HANDLE_VALUE; }
};

class IOCP
{
private:
    smart_handle handle_;

public:
    IOCP() : handle_(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
    {
        if (!handle_.is_valid())
        {
            throw std::runtime_error("CreateIoCompletionPort failed: " + std::to_string(GetLastError()));
        }
    }

    HANDLE get() const { return handle_.get(); }
    operator HANDLE() const { return handle_.get(); }
};

struct smart_overlapped : public OVERLAPPED
{
    std::function<void(int)> callback;
};

class ReadOnlyFile
{
private:
    smart_handle handle_;
    smart_handle iocp_;

public:
    ReadOnlyFile(const char *filename) : handle_(CreateFileA(
                                             filename,
                                             GENERIC_READ,
                                             0,
                                             nullptr,
                                             OPEN_EXISTING,
                                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                             nullptr)),
                                         iocp_(INVALID_HANDLE_VALUE)
    {
        if (!handle_.is_valid())
        {
            throw std::runtime_error("Failed to open file: " + std::to_string(GetLastError()));
        }
    }

    void associate_with_iocp(IOCP &iocp)
    {
        iocp_ = std::move(smart_handle(CreateIoCompletionPort(handle_.get(), iocp.get(), 0, 0)));
        if (!iocp_.is_valid())
        {
            throw std::runtime_error("CreateIoCompletionPort failed: " + std::to_string(GetLastError()));
        }
    }

    size_t size()
    {
        DWORD size = GetFileSize(handle_.get(), nullptr);
        if (size == INVALID_FILE_SIZE)
        {
            throw std::runtime_error("GetFileSize failed: " + std::to_string(GetLastError()));
        }
        return size;
    }

    class ReadAwaiter
    {
    private:
        int bytesRead_;
        char *buffer_;
        int bufferSize_;
        smart_overlapped overlapped_;
        smart_handle &handle_;

    public:
        ReadAwaiter(smart_handle &handle_, char *buffer, int bufferSize) : handle_(handle_), buffer_(buffer), bufferSize_(bufferSize)
        {
            ZeroMemory(&overlapped_, sizeof(overlapped_));
        }

        bool await_ready() const noexcept { return false; } // Always suspend

        bool await_suspend(std::coroutine_handle<> handle) noexcept
        {
            overlapped_.callback = [handle, this](int bytesRead)
            {
                bytesRead_ = bytesRead;
                handle.resume(); // Resume the coroutine when the read is complete
            };

            BOOL success = ReadFile(
                handle_,
                buffer_,
                bufferSize_,
                nullptr, // Must be null when using overlapped
                &overlapped_);

            if (!success && GetLastError() != ERROR_IO_PENDING)
            {
                std::cerr << "ReadFile failed: " << std::to_string(GetLastError()) << std::endl;
            }

            return !success; // Return true to suspend the coroutine
        }

        int await_resume() noexcept
        {
            return bytesRead_;
        }
    };

    async::task<std::string> read_all()
    {
        auto s = this->size(); // This is just to ensure the file is opened and ready for reading
        std::vector<char> buffer(s);

        co_await ReadAwaiter(handle_, buffer.data(), s);

        co_return std::string(buffer.data(), s);
    }

    HANDLE get() const { return handle_.get(); }
    operator HANDLE() const { return handle_.get(); }
};

async::task<void> print_file_contents(IOCP &iocp)
{

    ReadOnlyFile file0("helloworld.txt");

    file0.associate_with_iocp(iocp);
    fmt::println("File associated with IOCP");

    auto contents = co_await file0.read_all();
    fmt::println("File contents: {}", contents);

    ReadOnlyFile file1("2XC3-data/gutenberg/english/art_of_war.txt");

    file1.associate_with_iocp(iocp);
    fmt::println("File associated with IOCP");

    auto contents1 = co_await file1.read_all();
    fmt::println("File contents: {}", contents1);
}

void old_loop()
{

    IOCP iocp;
    ReadOnlyFile file("helloworld.txt");

    file.associate_with_iocp(iocp);

    fmt::println("File associated with IOCP");
    auto size = file.size(); // This is just to ensure the file is opened and ready for reading
    std::vector<char> buffer(size);

    // Step 3: Create context
    OVERLAPPED overlapped = {0};

    // Step 4: Start read
    DWORD bytesRead = 0;
    BOOL success = ReadFile(
        file,
        buffer.data(),
        buffer.size(),
        nullptr, // Must be null when using overlapped
        &overlapped);

    if (!success && GetLastError() != ERROR_IO_PENDING)
    {
        std::cerr << "ReadFile failed: " << GetLastError() << "\n";
        return;
    }

    // Step 5: Wait for completion
    DWORD transferred = 0;
    ULONG_PTR key;
    LPOVERLAPPED overlappedPtr = nullptr;

    async::event_signal signal;

    while (!signal.is_set())
    {
        if (GetQueuedCompletionStatus(iocp, &transferred, &key, &overlappedPtr, INFINITE))
        {
            if (transferred > 0)
            {
                std::cout << "Read " << transferred << " bytes:\n";
                fmt::println("Buffer: {}", std::string(buffer.data(), transferred));
                std::cout << "\n";
            }
            else
            {
                std::cout << "Read 0 bytes (EOF?)\n";
            }

            signal.set(); // Signal that the read is complete
        }
        else
        {
            std::cerr << "GetQueuedCompletionStatus failed: " << GetLastError() << "\n";
            break;
        }
    }
}

void new_loop(IOCP &iocp, async::event_signal &signal)
{

    DWORD transferred = 0;
    ULONG_PTR key;
    LPOVERLAPPED overlappedPtr = nullptr;

    while (!signal.is_set())
    {
        if (GetQueuedCompletionStatus(iocp, &transferred, &key, &overlappedPtr, INFINITE))
        {
            smart_overlapped *overlapped = static_cast<smart_overlapped *>(overlappedPtr);
            overlapped->callback(transferred); // Call the callback with the number of bytes read
        }
        else
        {
            std::cerr << "GetQueuedCompletionStatus failed: " << GetLastError() << "\n";
            break;
        }
    }
}

void run_app()
{
    fmt::println("Running on Windows");

    IOCP iocp;

    async::event_signal signal;

    auto task = print_file_contents(iocp);

    run_task_on_io(std::move(task), signal); // Start the task and associate it with the IOCP

    fmt::println("Task started");

    fmt::println("Running new loop");
    new_loop(iocp, signal); // Run the new loop to process IO events
    fmt::println("New loop finished");
}

#endif