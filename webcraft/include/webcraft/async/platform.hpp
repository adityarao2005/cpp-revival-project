#pragma once
#include <memory>

namespace webcraft::async
{
    namespace unsafe
    {
#ifdef _WIN32
#include <windows.h>

        using native_runtime_handle = HANDLE;

#elif defined(__linux__)

#ifndef IO_URING_QUEUE_SIZE
// @brief The size of the io_uring queue. This can be adjusted based on the application's needs.
#define IO_URING_QUEUE_SIZE 256
#endif

#include <unistd.h>
#include <liburing.h>

        using native_runtime_handle = io_uring;
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/event.h

        using native_runtime_handle = int; // kqueue file descriptor
#endif

        /// @brief Performs the initialization of the runtime handle based on the platform.
        /// This function is unsafe and should be used with caution. It is intended for internal use only.
        /// @param handle the runtime handle to initialize
        void initialize_runtime_handle(native_runtime_handle &handle);
        /// @brief Performs the destruction of the runtime handle based on the platform.
        /// This function is unsafe and should be used with caution. It is intended for internal use only.
        /// @param handle the runtime handle to destroy
        void destroy_runtime_handle(native_runtime_handle &handle);
    }

    class runtime_handle
    {
    private:
        unsafe::native_runtime_handle handle;

    public:
        runtime_handle()
        {
            unsafe::initialize_runtime_handle(handle);
        }

        ~runtime_handle()
        {
            unsafe::destroy_runtime_handle(handle);
        }

        unsafe::native_runtime_handle get() const
        {
            return handle;
        }

        runtime_handle(const runtime_handle &) = delete;
        runtime_handle(runtime_handle &&other) noexcept : handle(std::exchange(other.handle, nullptr)) {}
        runtime_handle &operator=(const runtime_handle &) = delete;
        runtime_handle &operator=(runtime_handle &&other) noexcept
        {
            if (this != &other)
            {
                unsafe::destroy_runtime_handle(handle);
                handle = std::exchange(other.handle, nullptr);
            }
            return *this;
        }
    };
}