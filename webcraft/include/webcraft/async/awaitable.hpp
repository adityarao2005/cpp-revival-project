#pragma once
#include <coroutine>
#include <utility>
#include <concepts>
#include <async/task.h>
#include <functional>

namespace webcraft::async
{

    /// \brief A concept that checks if the type T can be used to be returned from await_suspend
    template <typename T>
    concept suspend_type = std::same_as<T, std::coroutine_handle<>> ||
                           std::derived_from<T, std::coroutine_handle<>> || std::same_as<T, void> || std::same_as<T, bool>;

    /// \brief A concept that checks if the type T is an awaitable type
    template <typename T>
    concept Awaitable = requires(T t, std::coroutine_handle<> h) {
        { t.await_ready() } -> std::convertible_to<bool>;
        { t.await_suspend(h) } noexcept -> suspend_type;
        { t.await_resume() } noexcept;
    };

    template <typename T>
    using Task = ::async::task<T>;

}