#pragma once

#include "util/function_traits.hpp"
#include "util/type_list.hpp"
#include "util/variable_tarits.hpp"
#include <functional>
#include <string_view>
#include <utility>

namespace mirror {

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
struct basic_field_traits<T, false> : mirror::util::variable_traits<T> {
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

template <typename T, typename... Attrs>
struct field_traits
    : internal::basic_field_traits<T, mirror::util::is_function_v<T>> {
    constexpr field_traits(T&& pointer, std::string_view name, Attrs&&... attrs)
        : pointer_(std::forward<T>(pointer)),
          name_(get_name(name)),
          attrs_(std::forward<Attrs>(attrs)...) {}

    /**
     * @brief check whether field is a const member(class const function)
     */
    [[nodiscard]] constexpr bool is_const_member() const noexcept {
        return base::is_const_member();
    }

    /**
     * @brief check whether field is class member or static/global
     */
    [[nodiscard]] constexpr bool is_member() const noexcept {
        return base::is_member();
    }

    /**
     * @brief get field name
     */
    [[nodiscard]] constexpr std::string_view name() const noexcept {
        return name_;
    }

    /**
     * @brief get pointer
     */
    constexpr auto pointer() const noexcept { return pointer_; }

    /**
     * @brief get attributes
     */
    constexpr auto& attrs() const noexcept { return attrs_; }

    template <typename... Args>
    decltype(auto) invoke(Args&&... args) {
        if constexpr (!mirror::util::is_function_v<T>) {
            if constexpr (mirror::util::variable_traits<T>::is_member) {
                return std::invoke(this->pointer_, std::forward<Args>(args)...);
            } else {
                return *(this->pointer_);
            }

        } else {
            return std::invoke(this->pointer_, std::forward<Args>(args)...);
        }
    }

private:
    using base =
        internal::basic_field_traits<T, mirror::util::is_function_v<T>>;
    T pointer_;
    std::string_view name_;
    std::tuple<Attrs...> attrs_;
};
}  // namespace mirror