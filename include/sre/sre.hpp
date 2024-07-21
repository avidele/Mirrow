#pragma once

#include "util/function_traits.hpp"
#include "util/type_list.hpp"
#include "util/variable_tarits.hpp"
#include <string_view>

namespace mirrow {

namespace sre {
template <typename... Attrs>
using attr_list = util::type_list<Attrs...>;
}

namespace internal {
template <typename T, bool>
struct basic_field_traits;

template <typename T>
struct basic_field_traits<T, true> : mirror::util::function_traits<T> {
    [[nodiscard]] constexpr bool is_const() const noexcept {
        return mirror::util::function_traits<T>::is_const;
    }

    [[nodiscard]] constexpr bool is_member() const noexcept {
        return mirror::util::function_traits<T>::is_member;
    }

    [[nodiscard]] constexpr bool is_function() const noexcept { return true; }

    [[nodiscard]] constexpr bool is_variable() const noexcept { return false; }
};

template <typename T>
struct basic_field_traits<T, false> : mirror::util::variable_traits<T>{
    [[nodiscard]] constexpr bool is_member() const noexcept {
        return mirror::util::variable_traits<T>::is_member;
    }

    [[nodiscard]] constexpr bool is_const() const noexcept { return false; }

    [[nodiscard]] constexpr bool is_function() const noexcept { return false; }

    [[nodiscard]] constexpr bool is_variable() const noexcept { return true; }
};

}  // namespace internal

constexpr std::string_view get_name(std::string_view name) noexcept {
    if (auto i = name.find_last_of(':'); i != std::string_view::npos) {
        name = name.substr(i + 1);
    }
    if (auto i = name.find_last_of(':'); i != std::string_view::npos) {
        name = name.substr(i + 1);
    }
    if (auto i = name.find_first_of(')'); i != std::string_view::npos) {
        name = name.substr(0, i + 1);
    }
    return name;
}
}  // namespace mirrow