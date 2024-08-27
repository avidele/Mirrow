#pragma once
#include "dynamic/any.hpp"
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace mirror::dynamic {
class type {
public:
    template <typename derive_class>
    friend class meta_registry;
    virtual ~type();
    const std::string_view& get_name() const noexcept;
    const std::string_view& get_field(std::string_view key) const;
    std::string fetch_all_fields() const;
    bool has_field(std::string_view key) const;
    void set_field(std::string_view key, std::string_view value);

protected:
    explicit type(std::string_view name) : name_(name) {}

private:
    std::string_view name_;
    std::unordered_map<std::string_view, std::string_view> fields_;
};

class variable_type final : public type {
public:
    ~variable_type() override = default;

    template <typename T>
    void set_value(T value) const;

    void set_value(any* value) const;
    any get_value() const;

private:
    template <typename C>
    friend class class_registery;
    friend class global_registry;
    friend class global_context;
    friend class class_type;
    using setter = std::function<void(any*)>;
    using getter = std::function<any()>;

    struct construct_params {
        std::string_view name;
        size_t memory_size;
        const type_info* type_info_;
        setter setter_;
        getter getter_;
    };

    explicit variable_type(construct_params&& params);
    size_t memory_size_;
    const type_info* type_info_;
    setter setter_;
    getter getter_;
};

class function_type final : public type {
public:
    ~function_type() override = default;
    template <typename... Args>
    any call(Args&&... args) const;

    any call_with_size(any* args, uint8_t size) const;

private:
    template <typename C>
    friend class class_registery;
    friend class global_registry;
    friend class global_context;
    friend class class_type;

    using caller = std::function<any(any*, uint8_t)>;

    struct construct_params {
        std::string_view name;
        uint8_t arg_size;
        const type_info* return_type;
        std::vector<const type_info*> arg_types;
        caller caller_;
    };

    explicit function_type(construct_params&& params);
    uint8_t args_num_;
    const type_info* return_type_;
    std::vector<const type_info*> arg_types_;
    caller caller_;
};

class member_variable_type final : public type {
public:
    ~member_variable_type() override = default;

    template <typename C, typename T>
    void set_value(C&& object, T value) const;

    void set_value(any* object, any* value) const;
    any get_value(any* object) const;

private:
    friend class class_type;
    template <typename T>
    friend class class_registery;
    using setter = std::function<void(any*, any*)>;
    using getter = std::function<any(any*)>;

    struct construct_params {
        std::string_view name;
        uint32_t memory_size;
        const type_info* type_info_;
        setter setter_;
        getter getter_;
    };

    explicit member_variable_type(construct_params&& params);
    uint32_t memory_size_;
    const type_info* type_info_;
    setter setter_;
    getter getter_;
};

class member_function_type final : public type {
public:
    ~member_function_type() override = default;
    template <typename C, typename... Args>
    any call(C&& object, Args&&... args) const;

    any call_with_pointer(any* object, any* args, uint8_t size) const;

private:
    template <typename C>
    friend class class_registery;
    friend class class_type;
    using caller = std::function<any(any*, any*, uint8_t)>;

    struct construct_params {
        std::string_view name;
        uint8_t arg_size;
        const type_info* return_type;
        std::vector<const type_info*> arg_types;
        caller caller_;
    };

    explicit member_function_type(construct_params&& params);
    uint8_t args_num_;
    const type_info* return_type_;
    std::vector<const type_info*> arg_types_;
    caller caller_;
};

class constructor_type final : public type {
public:
    ~constructor_type() override = default;

    template <typename... Args>
    any construct_on_stack(Args&&... args) const;

    template <typename... Args>
    any new_object(Args&&... args) const;

private:
    friend class registry;
    friend class class_type;
    template <typename C>
    friend class class_registery;
    using caller = std::function<any(any*, uint8_t)>;

    struct constructor_params {
        std::string_view name;
        uint8_t args_num;
        std::vector<const type_info*> arg_types;
        caller stack_constructor_;
        caller heap_constructor_;
    };

    explicit constructor_type(constructor_params&& params);
    uint8_t args_num_;
    std::vector<const type_info*> arg_types_;
    caller stack_constructor_;
    caller heap_constructor_;
};

class destructor_type final : public type {
public:
    ~destructor_type() override = default;
    template <typename C>
    void call(C&& object) const;
    // void call_with_pointer(any* object) const;
private:
    friend class class_type;
    friend class registry;
    using caller = std::function<void(any*)>;

    struct construct_params {
        caller destructor_;
    };

    explicit destructor_type(construct_params&& params);
    caller destructor_;
};

class class_type final : public type {
public:
    ~class_type() override = default;

    template <typename C>
    requires std::is_class_v<C>
    static const class_type& get();

    static const class_type& get(const type_info* info);
    static const class_type& get(const std::string_view& name);
    static const class_type& get(size_t type_id);

private:
    friend class registry;
    template <typename C>
    friend class class_registery;
    static std::unordered_map<size_t, std::string_view> class_type_map_;
    using class_getter = std::function<const class_type*()>;

    struct construct_params {
        std::string_view name_;
        const type_info* type_info_;
        class_getter getter_;
        std::optional<std::function<any()>> default_object_creator;
        std::optional<destructor_type::construct_params> destructor_params;
        std::optional<constructor_type::constructor_params> default_constructor_params;
    };

    constructor_type& emplace_constructor(
        std::string_view name, constructor_type::constructor_params&& params);

    variable_type& emplace_static_variable(
        std::string_view name, variable_type::construct_params&& params);

    function_type& emplace_static_function(
        std::string_view name, function_type::construct_params&& params);

    member_variable_type& emplace_member_variable(
        std::string_view name, member_variable_type::construct_params&& params);

    member_function_type& emplace_member_function(
        std::string_view name, member_function_type::construct_params&& params);

    explicit class_type(construct_params&& params);
    void create_default_object(const std::function<any()>& constructor);

    const type_info* type_info_;
    class_getter getter_;
    std::optional<any> default_object_;
    std::optional<std::function<any()>> default_constructor_;
    std::optional<destructor_type> destructor_;
    std::unordered_map<std::string_view, constructor_type> constructors_;
    std::unordered_map<std::string_view, member_variable_type>
        member_variables_;
    std::unordered_map<std::string_view, member_function_type>
        member_functions_;
    std::unordered_map<std::string_view, variable_type> static_variables_;
    std::unordered_map<std::string_view, function_type> static_functions_;
};

class global_context final : public type {
public:
    ~global_context() override = default;
    static global_context& get_instance();
    variable_type& emplace_variable(std::string_view name,
                                    variable_type::construct_params&& params);
    const variable_type& get_variable(std::string_view name) const;

    function_type& emplace_function(std::string_view name,
                                    function_type::construct_params&& params);
    const function_type& get_function(std::string_view name) const;

private:
    global_context();
    std::unordered_map<std::string_view, variable_type> variables_;
    std::unordered_map<std::string_view, function_type> functions_;

    friend class registry;
    friend class global_registry;
};

}  // namespace mirror::dynamic

namespace mirror::dynamic {
template <typename T>
void variable_type::set_value(T value) const {
    any any_value = value;
    setter_(&any_value);
}

template <typename C>
requires std::is_class_v<C>
const class_type& class_type::get() {
    return get(get_type_info<C>());
}

template <typename... Args>
any function_type::call(Args&&... args) const {
    std::array<any, sizeof...(Args)> arguments = {
        any(std::forward<Args>(args))...};
    return call_with_size(arguments.data(), arguments.size());
}

template <typename C>
void destructor_type::call(C&& object) const {
    any any_object = any(std::forward<C>(object));
    destructor_(&any_object);
}

template <typename... Args>
any constructor_type::construct_on_stack(Args&&... args) const {
    std::array<any, sizeof...(Args)> arguments = {
        any(std::forward<Args>(args))...};
    return stack_constructor_(arguments.data(), arguments.size());
}

template <typename... Args>
any constructor_type::new_object(Args&&... args) const {
    std::array<any, sizeof...(Args)> arguments = {
        any(std::forward<Args>(args))...};
    return heap_constructor_(arguments.data(), arguments.size());
}

template <typename C, typename T>
void member_variable_type::set_value(C&& object, T value) const {
    any any_object = any(std::forward<C>(object));
    any any_value = any(std::forward<std::remove_reference_t<T>>);
    set_value(&any_object, &any_value);
}

template <typename C, typename... Args>
any member_function_type::call(C&& object, Args&&... args) const {
    std::array<any, sizeof...(Args)> arguments = {
        any(std::forward<Args>(args))...};
    any any_object = any(std::forward<C>(object));
    return caller_(&any_object, arguments.data(), arguments.size());
}
}  // namespace mirror::dynamic