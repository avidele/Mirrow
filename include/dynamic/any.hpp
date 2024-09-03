#pragma once
#include <algorithm>
#include <concepts>
#include <iostream>
#include <type_traits>

namespace mirror::dynamic{
   struct type_info {
        std::string name;
        size_t id;
        const bool is_const;
        const bool is_lvalue_reference;
        const bool is_rvalue_reference;
        const bool is_pointer;
        const bool is_class;
        const bool copy_constructible;
        const bool copy_assignable;
        const bool move_constructible;
        const bool move_assignable;
        size_t remove_pointer_type;
    };

    struct NamePresets {
        static constexpr auto global_scope = "_global_cope";
        static constexpr auto destructor = "_destructor";
        static constexpr auto default_constructor = "_default_constructor";
    };
    
    template <typename T>
    type_info* get_type_info();

    struct any_ops {
    using copy_fn = void*(*)(void*);
    using move_fn = void*(*)(void*);
    using destroy_fn = void(*)(void*);
    using assign_fn = bool(*)(void*, void*);

    copy_fn copy;
    move_fn move;
    destroy_fn destroy;
    assign_fn assign;
};

template <typename T>
any_ops& obtain_any_ops() {
    static any_ops ops = {
        [](void* data) -> void* {
            return new T(*static_cast<T*>(data));
        },
        [](void* data) -> void* {
            return new T(std::move(*static_cast<T*>(data)));
        },
        [](void* data) {
            delete static_cast<T*>(data);
        },
        [](void* data, void* other) -> bool {
            if constexpr(std::equality_comparable<T>) {
                return *static_cast<T*>(data) == *static_cast<T*>(other);
            } else {
                std::cout << "Type is not equality comparable" << std::endl;
                return false;
            }
        }
    };
    return ops;
}


class any {
public:
    any() = default;
    ~any();
    any(const any& other);
    any(any&& other) noexcept;

    template <typename T, typename Enable = std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::reference_wrapper<std::remove_reference_t<T>>>>>
    any(T&& other) noexcept;

    template <typename T>
    any(const std::reference_wrapper<T>& other) noexcept;

    template<typename T>
    any(std::reference_wrapper<T>&& other) noexcept;

    any& operator=(const any& other);
    any& operator=(any&& other) noexcept;
    template <typename T>
    any& operator=(T&& other);
    template <typename T>
    any& operator=(const std::reference_wrapper<T>& other);

    template<typename T>
    any& operator=(std::reference_wrapper<T>&& other);

    template<typename T>
    T& cast() const;

    [[nodiscard]] const void* data() const { return data_; }
private:
    template<typename T>
    void construct_value(T&& value);

    template<typename T>
    void construct_ref(const std::reference_wrapper<T>& value);
    void* data_ = nullptr;
    any_ops* ops_ = nullptr;
    const type_info* type_ = nullptr;
};

template <typename T>
type_info* get_type_info() {
    static type_info info = {
        .name = typeid(T).name(),
        .id = typeid(T).hash_code(),
        .is_const = std::is_const_v<T>,
        .is_lvalue_reference = std::is_lvalue_reference_v<T>,
        .is_rvalue_reference = std::is_rvalue_reference_v<T>,
        .is_pointer = std::is_pointer_v<T>,
        .is_class = std::is_class_v<T>,
        .copy_constructible = std::is_copy_constructible_v<T>,
        .copy_assignable = std::is_copy_assignable_v<T>,
        .move_constructible = std::is_move_constructible_v<T>,
        .move_assignable = std::is_move_assignable_v<T>,
        .remove_pointer_type = typeid(std::remove_pointer_t<T>).hash_code()
    };
    return &info;
}

template <typename T>
void any::construct_value(T&& value) {
    if (data_ != nullptr) {
        ops_->destroy(data_);
    }
    type_ = get_type_info<std::decay_t<T>>();
    data_ = new std::decay_t<T>(std::forward<T>(value));
    ops_ = &obtain_any_ops<std::decay<T>>();
}

template <typename T, typename Enable>
any::any(T&& other) noexcept
{
    construct_value(std::forward<T>(other));
}

template <typename T>
any& any::operator=(T&& other) {
    if (data_ != nullptr) {
        ops_->destroy(data_);
    }
    data_ = new std::decay_t<T>(std::forward<T>(other));
    type_ = get_type_info<std::decay_t<T>>();
    ops_ = &obtain_any_ops<std::decay_t<T>>();
    return *this;
}

template <typename T>
T& any::cast() const {
    if (ops_ == nullptr) {
        throw std::runtime_error("No data stored in any");
    }
    using non_ref_type = std::remove_reference_t<T>;
    if(type_->is_lvalue_reference){
        auto ref_wrap = static_cast<std::add_const_t<std::reference_wrapper<std::remove_reference_t<T>>>*>(data_);
        // std::cout << "Reference Wrapper Address: " << &(ref_wrap->get()) << std::endl;
        return ref_wrap->get();
    }
    return *static_cast<non_ref_type*>(data_);
}

template<typename T>
any::any(std::reference_wrapper<T>&& other) noexcept {
    construct_ref(std::move(other));
}

template <typename T>
void any::construct_ref(const std::reference_wrapper<T>& value) {
    type_ = get_type_info<T&>();
    data_ = new std::reference_wrapper<T>(value);
    ops_ = &obtain_any_ops<std::reference_wrapper<T>>();
}

template <typename T>
any::any(const std::reference_wrapper<T>& other) noexcept {
    construct_ref(other);
}

template<typename T>
any& any::operator=(std::reference_wrapper<T>&& other) {
    if (data_ != nullptr) {
        ops_->destroy(data_);
        ops_ = nullptr;
    }

    construct_ref(std::move(other));
    return *this;
}

template<typename T>
any& any::operator=(const std::reference_wrapper<T>& other) {
    if (data_ != nullptr) {
        ops_->destroy(data_);
    }
    construct_ref(other);
    return *this;
}
} // namespace mirror::dynamic

