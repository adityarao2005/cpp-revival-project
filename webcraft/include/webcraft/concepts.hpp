#pragma once

#include <concepts>

namespace webcraft
{
    template <typename T, typename R>
    concept not_same_as = !std::same_as<T, R>;

    template <bool T>
    concept not_true = !T;
}