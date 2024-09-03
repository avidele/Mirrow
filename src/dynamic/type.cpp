#include "dynamic/type.hpp"
#include "dynamic/any.hpp"
#include "dynamic/register.hpp"
#include <stdexcept>
#include <string_view>
using namespace mirror::dynamic;

type::~type() = default;

std::unordered_map<size_t, std::string_view> class_type::class_type_map_{}; 

const std::string_view& type::get_name() const noexcept {
    return name_;
}

const std::string_view& type::get_field(std::string_view key) const {
    if (!has_field(key)) {
        throw std::runtime_error("Field not found");
    }
    return fields_.at(key);
}

void type::set_field(std::string_view key, std::string_view value) {
    fields_[key] = value;
}

std::string type::fetch_all_fields() const {
    std::string result;
    for (const auto& [key, value] : fields_) {
        result += std::string(key);
        result += ": ";
        result += value;
        result += "\n";
    }
    return result;
}

bool type::has_field(std::string_view key) const {
    return fields_.contains(key);
}

void variable_type::set_value(any* value) const {
    setter_(value);
}

any variable_type::get_value() const {
    return getter_();
}

any function_type::call_with_size(any* args, uint8_t size) const {
    return caller_(args, size);
}

function_type::function_type(construct_params&& params)
    : type(params.name),
      return_type_(params.return_type),
      arg_types_(params.arg_types),
      caller_(params.caller_),
      args_num_(params.arg_size) {}

const class_type& class_type::get(const type_info* info) {
    return get(info->id);
}

const class_type& class_type::get(size_t type_id) {
    const auto iter = class_type_map_.find(type_id);
    if(iter == class_type_map_.end()){
        throw std::runtime_error("Type not found");
    }
    return get(iter->second);
}

const class_type& class_type::get(const std::string_view& name) {
    const auto& classes = registry::instance().classes_;
    const auto iter = classes.find(name);
    if (iter == classes.end()) {
        throw std::runtime_error("Type not found");
    }
    return iter->second;
}

void class_type::create_default_object(const std::function<any()>& constructor) {
    default_object_ = constructor();
}

class_type::class_type(construct_params&& params)
    : type(params.name_),
      type_info_(params.type_info_),
      getter_(params.getter_) {
        if (params.default_object_creator.has_value()) {
            create_default_object(params.default_object_creator.value());
        }
        if(params.destructor_params.has_value()){
            destructor_ = destructor_type(std::move(params.destructor_params.value()));
        }
        if(params.default_constructor_params.has_value()){
            emplace_constructor(NamePresets::default_constructor, std::move(params.default_constructor_params.value()));
        }
      }

constructor_type& class_type::emplace_constructor(
    std::string_view name, constructor_type::constructor_params&& params) {
    constructors_.emplace(name, constructor_type(std::move(params)));
    return constructors_.at(name);
}

variable_type& class_type::emplace_static_variable(
    std::string_view name, variable_type::construct_params&& params) {
    static_variables_.emplace(name, variable_type(std::move(params)));
    return static_variables_.at(name);
}

function_type& class_type::emplace_static_function(
    std::string_view name, function_type::construct_params&& params) {
    static_functions_.emplace(name, function_type(std::move(params)));
    return static_functions_.at(name);
}

member_variable_type::member_variable_type(construct_params&& params)
    : type(params.name),
      memory_size_(params.memory_size),
      type_info_(params.type_info_),
      setter_(params.setter_),
      getter_(params.getter_) {}

member_variable_type& class_type::emplace_member_variable(
    std::string_view name, member_variable_type::construct_params&& params) {
    member_variables_.emplace(name, member_variable_type(std::move(params)));
    return member_variables_.at(name);
}

member_function_type::member_function_type(construct_params&& params)
    : type(params.name),
      return_type_(params.return_type),
      arg_types_(params.arg_types),
      caller_(params.caller_),
      args_num_(params.arg_size) {}

member_function_type& class_type::emplace_member_function(
    std::string_view name, member_function_type::construct_params&& params) {
    member_functions_.emplace(name, member_function_type(std::move(params)));
    return member_functions_.at(name);
}

const variable_type& class_type::get_static_variable(std::string_view name) const {
    const auto iter = static_variables_.find(name);
    if (iter == static_variables_.end()) {
        throw std::runtime_error("Variable not found");
    }
    return iter->second;
}

const function_type& class_type::get_static_function(std::string_view name) const {
    const auto iter = static_functions_.find(name);
    if (iter == static_functions_.end()) {
        throw std::runtime_error("Function not found");
    }
    return iter->second;
}

const member_variable_type& class_type::get_member_variable(std::string_view name) const {
    const auto iter = member_variables_.find(name);
    if (iter == member_variables_.end()) {
        throw std::runtime_error("Variable not found");
    }
    return iter->second;
}

const constructor_type& class_type::get_constructor(std::string_view name) const {
    const auto iter = constructors_.find(name);
    if (iter == constructors_.end()) {
        throw std::runtime_error("Constructor not found");
    }
    return iter->second;
}

const member_function_type& class_type::get_member_function(std::string_view name) const {
    const auto iter = member_functions_.find(name);
    if (iter == member_functions_.end()) {
        throw std::runtime_error("Function not found");
    }
    return iter->second;
}

const destructor_type& class_type::get_destructor() const {
    if(!destructor_.has_value()){
        throw std::runtime_error("Destructor not found");
    }
    return destructor_.value();
}

variable_type::variable_type(construct_params&& params)
    : type(params.name),
      memory_size_(params.memory_size),
      type_info_(params.type_info_),
      setter_(params.setter_),
      getter_(params.getter_) {}

variable_type& global_context::emplace_variable(
    std::string_view name, variable_type::construct_params&& params) {
    variables_.emplace(name, variable_type(std::move(params)));
    return variables_.at(name);
}

global_context::global_context()
    : type(std::string_view(NamePresets::global_scope)) {}

global_context& global_context::get_instance() {
    return registry::instance().global_context_;
}

const variable_type& global_context::get_variable(std::string_view name) const {
    const auto iter = variables_.find(name);
    if (iter == variables_.end()) {
        throw std::runtime_error("Variable not found");
    }
    return iter->second;
}

function_type& global_context::emplace_function(
    std::string_view name, function_type::construct_params&& params) {
    functions_.emplace(name, function_type(std::move(params)));
    return functions_.at(name);
}

const function_type& global_context::get_function(std::string_view name) const {
    const auto iter = functions_.find(name);
    if (iter == functions_.end()) {
        throw std::runtime_error("Function not found");
    }
    return iter->second;
}

destructor_type::destructor_type(construct_params&& params)
    : type(std::string_view(NamePresets::destructor)),
      destructor_(std::move(params.destructor_)) {}

constructor_type::constructor_type(constructor_params&& params)
    : type(params.name),
        arg_types_(params.arg_types),
        args_num_(params.args_num),
        stack_constructor_(params.stack_constructor_),
        heap_constructor_(params.heap_constructor_) {}
      
any member_variable_type::get_value(any* object) const {
    return getter_(object);
}

void member_variable_type::set_value(any* object, any* value) const {
    setter_(object, value);
}

any enum_item::get() const{
    return getter_();
}

bool enum_item::compare(any* other) const {
    return comparer_(other);
}

enum_item::enum_item(construct_params&& params)
    : type(params.name),
      getter_(params.getter_),
      comparer_(params.comparer_) {}

enum_type::enum_type(construct_params&& params)
    : type(params.name_),
      type_info_(params.type_info_) {}

enum_item& enum_type::emplace_item(std::string_view name, enum_item::construct_params&& params) {
    enum_values_.emplace(name, enum_item(std::move(params)));
    return enum_values_.at(name);
}

const enum_type& enum_type::get(std::string_view name) {
    const auto& enums = registry::instance().enums_;
    const auto iter = enums.find(name);
    if (iter == enums.end()) {
        throw std::runtime_error("Enum not found");
    }
    return iter->second;
}

const enum_item& enum_type::get_item(std::string_view name) const {
    const auto iter = enum_values_.find(name);
    if (iter == enum_values_.end()) {
        throw std::runtime_error("Enum item not found");
    }
    return iter->second;
}