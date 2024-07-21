#pragma once
#include <cstddef>

namespace mirrow::util {
template <typename... Ts>
struct type_list {
    using self = type_list<Ts...>;
    static constexpr size_t size = sizeof...(Ts);
};

namespace detail {
template <typename T, size_t size>
struct type_list_get;

template <template <typename...> typename ListType, typename T, typename... Ts,
          size_t size>
struct type_list_get<ListType<T, Ts...>, size>
    : type_list_get<ListType<Ts...>, size - 1> {};

template <template <typename...> typename ListType, typename T, typename... Ts>
struct type_list_get<ListType<T, Ts...>, 0>{
    using type = T;
};

template <typename T>
struct list_size;

template <template <typename...> typename ListType, typename... Ts>
struct list_size<ListType<Ts...>>
{
    static constexpr size_t value = sizeof...(Ts);
};

template <typename T>
struct get_list_head;

template <template <typename...> typename ListType, typename T, typename... Ts>
struct get_list_head<ListType<T, Ts...>>{
    using type = T;
};

template <typename List, typename T>
struct type_list_push_front;

template <template <typename...> typename ListType, typename T, typename... Ts>
struct type_list_push_front<ListType<Ts...>, T>{
    using type = ListType<T, Ts...>;
};
}  // namespace detail

template <typename List, size_t Size>
using type_list_get = typename detail::type_list_get<List, Size>::type;

template <typename List>
constexpr size_t list_size = detail::list_size<List>::value;

template <typename List>
using get_list_head = typename detail::get_list_head<List>::type;

template <typename List, typename T>
using type_list_push_front = typename detail::type_list_push_front<List, T>::type;

}  // namespace mirrow::util