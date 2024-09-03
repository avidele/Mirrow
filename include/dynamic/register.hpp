#pragma once
#include "dynamic/type.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <util/function_traits.hpp>
#include <util/variable_tarits.hpp>

namespace mirror::dynamic {
template <typename derive_class>
class meta_registry {
public:
    virtual ~meta_registry() = default;
    derive_class& meta_data(std::string_view key, std::string_view value);

protected:
    derive_class& set_context(type* context);

    explicit meta_registry(type* context) : context(context) {}

private:
    type* context;
};

template <typename C>
class class_registery final : public meta_registry<class_registery<C>> {
public:
    ~class_registery() override = default;

    template <typename... Args>
    class_registery& constructor(std::string_view name);

    template <auto Ptr>
    class_registery& static_variable(std::string_view name);

    template <auto Ptr>
    class_registery& static_function(std::string_view name);

    template <auto Ptr>
    class_registery& member_variable(std::string_view name);

    template <auto Ptr>
    class_registery& member_function(std::string_view name);

private:
    friend class registry;
    explicit class_registery(class_type& clazz);
    class_type& clazz;
};

template <typename C>
class enum_registry final : public meta_registry<enum_registry<C>> {
public:
    ~enum_registry() override = default;
    enum_registry& item(std::string_view name, C value);

private:
    friend class registry;
    explicit enum_registry(enum_type& enum_type);
    enum_type& enum_type_;
};

class global_registry final : public meta_registry<global_registry> {
public:
    ~global_registry() override = default;

    template <auto ptr>
    global_registry& function(std::string_view name);

    template <auto ptr>
    global_registry& variable(std::string_view name);

private:
    friend class registry;
    explicit global_registry(global_context& global_context);
    global_context& global_context_;
};

class registry {
public:
    static registry& instance();
    ~registry() = default;
    global_registry global();

    template <typename C, typename B = void>
    requires std::is_class_v<C> &&
             (std::is_void_v<B> || std::is_base_of_v<B, C>)
    class_registery<C> Class(std::string_view name);

    template <typename C>
    requires std::is_enum_v<C>
    enum_registry<C> Enum(std::string_view name);

private:
    registry() noexcept;
    std::unordered_map<std::string_view, class_type> classes_;
    std::unordered_map<std::string_view, enum_type> enums_;
    global_context global_context_;

    class_type& emplace_class(std::string_view name,
                              class_type::construct_params&& params);

    enum_type& emplace_enum(std::string_view name,
                            enum_type::construct_params&& params);
    friend class global_context;
    friend class class_type;
    friend class enum_type;
};
}  // namespace mirror::dynamic

namespace mirror::dynamic {
template <typename derive_class>
derive_class& meta_registry<derive_class>::meta_data(std::string_view key,
                                                     std::string_view value) {
    if (context) {
        context->set_field(key, value);
    }
    return static_cast<derive_class&>(*this);
}

template <typename derive_class>
derive_class& meta_registry<derive_class>::set_context(type* context) {
    this->context = context;
    return static_cast<derive_class&>(*this);
}

template <auto ptr>
global_registry& global_registry::variable(std::string_view name) {
    using value_type =
        typename mirror::util::variable_pointer_traits<ptr>::type;
    // const auto iter = global_context_.variables_.find(name);
    variable_type::construct_params params;
    params.name = name;
    params.memory_size = sizeof(value_type);
    params.type_info_ = get_type_info<value_type>();
    params.setter_ = [](const any* value) { *ptr = value->cast<value_type>(); };
    params.getter_ = []() -> any { return any(*ptr); };
    return set_context(
        &global_context_.emplace_variable(name, std::move(params)));
}

template <typename args_tuple, size_t... I>
auto get_arg_type_info(std::index_sequence<I...>) {
    return std::vector<const type_info*>{
        get_type_info<std::tuple_element_t<I, args_tuple>>()...};
}

template <typename args_tuple, size_t... I>
auto cast_array_to_args_tuple(any* args, std::index_sequence<I...>) {
    return args_tuple{args[I].cast<std::tuple_element_t<I, args_tuple>>()...};
}

template <auto ptr>
global_registry& global_registry::function(std::string_view name) {
    using args_type =
        typename mirror::util::function_traits<decltype(ptr)>::args;
    using ret_type =
        typename mirror::util::function_traits<decltype(ptr)>::return_type;

    constexpr size_t args_size = std::tuple_size_v<args_type>;
    function_type::construct_params params;
    params.name = name;
    params.arg_size = args_size;
    params.return_type = get_type_info<ret_type>();
    params.arg_types =
        get_arg_type_info<args_type>(std::make_index_sequence<args_size>{});
    params.caller_ = [](any* args, uint8_t size) -> any {
        auto args_tuple = cast_array_to_args_tuple<args_type>(
            args, std::make_index_sequence<args_size>{});
        if constexpr (std::is_void_v<ret_type>) {
            std::apply(ptr, args_tuple);
            return {};
        } else {
            return any(std::apply(ptr, args_tuple));
        }
    };
    return set_context(
        &global_context_.emplace_function(name, std::move(params)));
}

template <typename C, typename B>
requires std::is_class_v<C> && (std::is_void_v<B> || std::is_base_of_v<B, C>)
class_registery<C> registry::Class(std::string_view name) {
    class_type::construct_params params;
    params.name_ = name;
    params.type_info_ = get_type_info<C>();
    params.getter_ = []() -> const class_type* {
        if constexpr (std::is_void_v<B>) {
            return nullptr;
        } else {
            return &class_type::get<B>();
        }
    };
    if constexpr (std::is_default_constructible_v<C>) {
        params.default_object_creator = []() -> any { return any(C()); };
    }
    if constexpr (std::is_destructible_v<C>) {
        destructor_type::construct_params destructor_params;
        destructor_params.destructor_ = [](any* object) {
            object->cast<C&>().~C();
        };
        params.destructor_params = std::move(destructor_params);
    }
    if constexpr (std::is_default_constructible_v<C>) {
        constructor_type::constructor_params constructor_params;
        constructor_params.name = NamePresets::default_constructor;
        constructor_params.args_num = 0;
        constructor_params.stack_constructor_ =
            [](any* args, uint8_t size) -> any { return any(C()); };
        constructor_params.heap_constructor_ =
            [](any* args, uint8_t size) -> any { return any(new C()); };
        params.default_constructor_params = std::move(constructor_params);
    }
    class_type::class_type_map_[params.type_info_->id] = name;
    return class_registery<C>(emplace_class(name, std::move(params)));
}

template <typename C>
class_registery<C>::class_registery(class_type& clazz)
    : clazz(clazz), meta_registry<class_registery<C>>(&clazz) {}

template <typename C>
template <typename... Args>
class_registery<C>& class_registery<C>::constructor(std::string_view name) {
    using args_tuple_t = std::tuple<Args...>;
    constexpr size_t args_size = std::tuple_size_v<args_tuple_t>;
    constructor_type::constructor_params params;
    params.name = name;
    params.args_num = sizeof...(Args);
    params.arg_types = std::vector<const type_info*>{get_type_info<Args>()...};
    params.stack_constructor_ = [](any* args, uint8_t size) -> any {
        auto args_tuple = cast_array_to_args_tuple<args_tuple_t>(
            args, std::make_index_sequence<args_size>{});
        return any(std::apply(
            [](auto&&... args) -> C {
                return C(std::forward<decltype(args)>(args)...);
            },
            args_tuple));
    };
    params.heap_constructor_ = [](any* args, uint8_t size) -> any {
        auto args_tuple = cast_array_to_args_tuple<args_tuple_t>(
            args, std::make_index_sequence<args_size>{});
        return any(std::apply(
            [](auto&&... args) -> C* {
                return new C(std::forward<decltype(args)>(args)...);
            },
            args_tuple));
    };
    return meta_registry<class_registery>::set_context(
        &clazz.emplace_constructor(name, std::move(params)));
}

template <typename C>
template <auto Ptr>
class_registery<C>& class_registery<C>::static_variable(std::string_view name) {
    using value_type =
        typename mirror::util::variable_pointer_traits<Ptr>::type;
    variable_type::construct_params params;
    params.name = name;
    params.memory_size = sizeof(value_type);
    params.type_info_ = get_type_info<value_type>();
    params.setter_ = [](const any* value) { *Ptr = value->cast<value_type>(); };
    params.getter_ = []() -> any { return any(std::ref(*Ptr)); };
    return meta_registry<class_registery>::set_context(
        &clazz.emplace_static_variable(name, std::move(params)));
}

template <typename C>
template <auto Ptr>
class_registery<C>& class_registery<C>::static_function(std::string_view name) {
    using args_type =
        typename mirror::util::function_traits<decltype(Ptr)>::args;
    using ret_type =
        typename mirror::util::function_traits<decltype(Ptr)>::return_type;

    constexpr size_t args_size = std::tuple_size_v<args_type>;
    function_type::construct_params params;
    params.name = name;
    params.arg_size = args_size;
    params.return_type = get_type_info<ret_type>();
    params.arg_types =
        get_arg_type_info<args_type>(std::make_index_sequence<args_size>{});
    params.caller_ = [](any* args, uint8_t size) -> any {
        auto args_tuple = cast_array_to_args_tuple<args_type>(
            args, std::make_index_sequence<args_size>{});
        if constexpr (std::is_void_v<ret_type>) {
            std::apply(Ptr, args_tuple);
            return {};
        } else {
            return any(std::apply(Ptr, args_tuple));
        }
    };
    return meta_registry<class_registery>::set_context(
        &clazz.emplace_static_function(name, std::move(params)));
}

template <typename C>
template <auto Ptr>
class_registery<C>& class_registery<C>::member_variable(std::string_view name) {
    using class_type =
        typename mirror::util::variable_pointer_traits<Ptr>::class_t;
    using value_type =
        typename mirror::util::variable_pointer_traits<Ptr>::type;
    member_variable_type::construct_params params;
    params.name = name;
    params.memory_size = sizeof(value_type);
    params.type_info_ = get_type_info<value_type>();
    params.setter_ = [](const any* object, const any* value) {
        object->cast<class_type&>().*Ptr = value->cast<value_type>();
    };
    params.getter_ = [](const any* object) -> any {
        return any(std::ref(object->cast<class_type&>().*Ptr));
    };
    return meta_registry<class_registery>::set_context(
        &clazz.emplace_member_variable(name, std::move(params)));
}

template <typename C>
template <auto Ptr>
class_registery<C>& class_registery<C>::member_function(std::string_view name) {
    using class_type =
        typename mirror::util::function_traits<decltype(Ptr)>::class_type;
    using args_type =
        typename mirror::util::function_traits<decltype(Ptr)>::args;
    using ret_type =
        typename mirror::util::function_traits<decltype(Ptr)>::return_type;

    constexpr size_t args_size = std::tuple_size_v<args_type>;
    member_function_type::construct_params params;
    params.name = name;
    params.arg_size = args_size;
    params.return_type = get_type_info<ret_type>();
    params.arg_types =
        get_arg_type_info<args_type>(std::make_index_sequence<args_size>{});
    params.caller_ = [](any* object, any* args, uint8_t size) -> any {
        auto args_tuple = cast_array_to_args_tuple<args_type>(
            args, std::make_index_sequence<args_size>{});
        if constexpr (std::is_void_v<ret_type>) {
            std::apply(Ptr, std::tuple_cat(
                                std::make_tuple(object->cast<class_type&>()),
                                args_tuple));
            return {};
        } else {
            return any(std::apply(
                Ptr,
                std::tuple_cat(std::make_tuple(object->cast<class_type&>()),
                               args_tuple)));
        }
    };
    return meta_registry<class_registery>::set_context(
        &clazz.emplace_member_function(name, std::move(params)));
}

template <typename C>
enum_registry<C>::enum_registry(enum_type& enum_type)
    : enum_type_(enum_type), meta_registry<enum_registry<C>>(&enum_type) {}

template <typename C>
requires std::is_enum_v<C> 
enum_registry<C> registry::Enum(std::string_view name) {
    enum_type::construct_params params;
    params.name_ = name;
    params.type_info_ = get_type_info<C>();
    return enum_registry<C>(emplace_enum(name, std::move(params)));
}

template <typename C>
enum_registry<C>& enum_registry<C>::item(std::string_view name, C value) {
    enum_item::construct_params params;
    params.name = name;
    params.getter_ = [value]() -> any { return any(value); };
    params.comparer_ = [value](any* other) -> bool {
        return value == other->cast<C>();
    };
    return meta_registry<enum_registry>::set_context(
        &enum_type_.emplace_item(name, std::move(params)));
}
}  // namespace mirror::dynamic