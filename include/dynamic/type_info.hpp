#pragma once
#include <cstddef>
#include <optional>
#include <string_view>

namespace mirror::dynamic {
using type_id = size_t;

enum class value_category {
    None,
    Boolean,
    Numeric,
    String,
    Enum,
    Class,
    Array,
    Pointer,
    Property,
    Optional,
};

template <typename T>
value_category get_category() {
    if constexpr (std::is_same_v<T, bool>) {
        return value_category::Boolean;
    } else if constexpr (std::is_arithmetic_v<T>) {
        return value_category::Numeric;
    } else if constexpr (std::is_same_v<T, std::string> ||
                         std::is_same_v<T, std::string_view>) {
        return value_category::String;
    } else if (std::is_enum_v<T>) {
        return value_category::Enum;
    } else if (std::is_class_v<T>) {
        return value_category::Class;
    } else if (std::is_array_v<T>) {
        return value_category::Array;
    } else if (std::is_pointer_v<T>) {
        return value_category::Pointer;
    } else if (std::is_same_v<T, std::optional<T>>) {
        return value_category::Optional;
    } else if (std::is_same_v<T, std::pair<std::string_view, T>>) {
        return value_category::Property;
    } else {
        return value_category::None;
    }
}

struct type_info {
    virtual ~type_info() = default;
    virtual type_id id() const noexcept{ return id_; }
    virtual const char* name() const noexcept{ return name_.data(); }
    virtual value_category category() const noexcept{ return category_; }

    type_info(std::string_view name, type_id id, value_category category)
        : name_(name), id_(id), category_(category) {}

    std::string_view name_;
    type_id id_;
    const value_category category_;
};
}  // namespace mirror::dynamic