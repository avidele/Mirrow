#pragma once

#include <type_traits>

namespace mirror::util {
namespace detail {

template <typename T>
struct variable_type {
    using type = T;
};

template <typename Class, typename T>
struct variable_type<T Class::*> {
    using type = T;
};

}  // namespace detail

template <typename T>
using variable_type_t = typename detail::variable_type<T>::type;

namespace internal {
template <typename T>
struct basic_variable_traits {
    using type = variable_type_t<T>;
    static constexpr bool is_member = std::is_member_pointer_v<T>;
};

}  // namespace internal
template <typename T>
struct variable_traits;

template <typename T>
struct variable_traits<T *> : internal::basic_variable_traits<T> {
    using pointer = T *;
};

template <typename Class, typename T>
struct variable_traits<T Class::*>
    : internal::basic_variable_traits<T Class::*> {
    using pointer = T Class::*;
    using class_t = Class;
};

namespace detail {
template <auto V>
struct variable_pointer_traits : variable_traits<decltype(V)> {};
}  // namespace detail

template <auto V>
using variable_pointer_traits = detail::variable_pointer_traits<V>;

}