#pragma once
#include <cstddef>

namespace mirror::util {
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
struct type_list_get<ListType<T, Ts...>, 0> {
    using type = T;
};

template <typename T>
struct list_size;

template <template <typename...> typename ListType, typename... Ts>
struct list_size<ListType<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

template <typename T>
struct get_list_head;

template <template <typename...> typename ListType, typename T, typename... Ts>
struct get_list_head<ListType<T, Ts...>> {
    using type = T;
};

template <typename T>
struct get_list_tail;

template <template <typename...> typename ListType, typename T, typename... Ts>
struct get_list_tail<ListType<T, Ts...>> {
    using type = ListType<Ts...>;
};

template <typename List, typename T>
struct type_list_push_front;

template <template <typename...> typename ListType, typename T, typename... Ts>
struct type_list_push_front<ListType<Ts...>, T> {
    using type = ListType<T, Ts...>;
};
}  // namespace detail

template <typename List, size_t Size>
using type_list_get = typename detail::type_list_get<List, Size>::type;

template <typename List>
constexpr size_t list_size = detail::list_size<List>::value;

template <typename List>
using get_list_head = typename detail::get_list_head<List>::type;

template <typename List>
using get_list_tail = typename detail::get_list_tail<List>::type;

template <typename List, typename T>
using type_list_push_front =
    typename detail::type_list_push_front<List, T>::type;

namespace detail {
template <typename List, size_t Size, template <typename> typename F>
struct apply_func {
    using type = F<util::type_list_get<List, Size>>;
};

template <typename List, template <typename> typename F>
struct foreach_list {};

template <template <typename...> typename ListType,
          template <typename> typename F, typename... Ts>
struct foreach_list<ListType<Ts...>, F> {
    using type = ListType<typename F<Ts>::type...>;
};

template <typename List, template <typename> typename F>
struct disjunction {
    static constexpr bool value =
        F<util::get_list_head<List>>::value ||
        disjunction<util::get_list_tail<List>, F>::value;
};

template <template <typename...> typename ListType,
          template <typename> typename F>
struct disjunction<ListType<>, F> {
    static constexpr bool value = false;
};
}  // namespace detail

template <typename List, size_t Size, template <typename> typename F>
using apply_func_t = typename detail::apply_func<List, Size, F>::type;

template <typename List, template <typename> typename F>
using foreach_list_t = typename detail::foreach_list<List, F>::type;

template <typename List, template <typename> typename F>
constexpr bool disjunction_v = detail::disjunction<List, F>::value;
}  // namespace mirror::util
