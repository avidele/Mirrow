#include "dynamic/register.hpp"
#include "dynamic/type.hpp"
#include <cstdio>

using namespace mirror::dynamic;

global_registry::global_registry(global_context& global_context)
    : meta_registry(&global_context), global_context_(global_context) {}

registry::registry() noexcept = default;

registry& registry::instance() {
    static registry instance;
    return instance;
}

global_registry registry::global() {
    return global_registry(global_context_);
}

class_type& registry::emplace_class(std::string_view name, class_type::construct_params&& params) {
    classes_.emplace(name, class_type(std::move(params)));
    return classes_.at(name);
}

enum_type& registry::emplace_enum(std::string_view name, enum_type::construct_params&& params) {
    enums_.emplace(name, enum_type(std::move(params)));
    return enums_.at(name);
}