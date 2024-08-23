#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "dynamic/any.hpp"

namespace mirror::dynamic {
class type {
public:
    virtual ~type();
    const std::string_view& get_name() const noexcept;
    const std::string_view& get_field(std::string_view key) const;
    std::string fetch_all_fields() const;
    bool has_field(std::string_view key) const;

private:
    std::string_view name_;
    std::unordered_map<std::string_view, std::string_view> fields_;
};

class viriable_type final: public type {
    public:
    ~viriable_type() override = default;

    template<typename T>
    void set_value(T value) const;

    

    private:
    using setter = std::function<void(any*)>;

    setter setter_;
};
}  // namespace mirror::dynamic