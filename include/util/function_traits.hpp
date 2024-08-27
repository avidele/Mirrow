#pragma once

#include <tuple>
#include <type_traits>
namespace mirror::util {
namespace detail {
template <typename T>
struct function_type;

template <typename Ret, typename... Args>
struct function_type<Ret(Args...)> {
    using type = Ret(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_type<Ret (Class::*)(Args...)> {
    using type = Ret (Class::*)(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_type<Ret (Class::*)(Args...) const> {
    using type = Ret (Class::*)(Args...) const;
};
}  // namespace detail

template <typename T>
using function_type_t = typename detail::function_type<T>::type;

namespace detail{
    template <typename Func>
    struct basic_function_traits;

    template <typename Ret, typename... Args>
    struct basic_function_traits<Ret(Args...)> {
        using return_type = Ret;
        using args = std::tuple<Args...>;
    };
}

template <typename Func>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)>: detail::basic_function_traits<Ret(Args...)> {
    using type = Ret(*)(Args...);
    static constexpr bool is_member = false;
    static constexpr bool is_const = false;
};

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)> : detail::basic_function_traits<Ret(Args...)>
{
    using type = Ret(Args...);
    using pointer = Ret (*)(Args...);
    static constexpr bool is_member = false;
    static constexpr bool is_const = false;
};

template <typename Ret, typename... Args, typename Class>
struct function_traits<Ret(Class::*)(Args...)> : detail::basic_function_traits<Ret(Args...)>
{
    using type = Ret(Class::*)(Args...);
    using pointer = Ret (Class::*)(Args...);
    using class_type = Class;    
    static constexpr bool is_member = true;
    static constexpr bool is_const = false;
};

template <typename Ret, typename... Args, typename Class>
struct function_traits<Ret(Class::*)(Args...) const> : detail::basic_function_traits<Ret(Args...) const>
{
    using type = Ret(Class::*)(Args...) const;
    using pointer = Ret (Class::*)(Args...) const;
    using class_type = Class;  
    static constexpr bool is_member = true;
    static constexpr bool is_const = true;
};

template <typename T>
constexpr bool is_function_v = std::is_function_v<T> || std::is_member_pointer_v<T>;
}  // namespace mirror::util
