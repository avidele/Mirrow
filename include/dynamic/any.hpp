#pragma once
#include <algorithm>
#include <concepts>
#include <iostream>
#include <type_traits>

namespace mirror::dynamic{

   struct type_info {
        std::string name;
        size_t id;
        const bool isConst;
        const bool isLValueReference;
        const bool isRValueReference;
        const bool isPointer;
        const bool isClass;
        const bool copyConstructible;
        const bool copyAssignable;
        const bool moveConstructible;
        const bool moveAssignable;
        size_t removePointerType;
    };
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

    template <typename T>
    any(T&& other) noexcept;

    any& operator=(const any& other);
    any& operator=(any&& other) noexcept;
    template <typename T>
    any& operator=(T&& other);

    template<typename T>
    T& cast();

    [[nodiscard]] const void* data() const { return data_; }
private:
    void* data_ = nullptr;
    any_ops* ops_ = nullptr;
};

template <typename T>
any::any(T&& other) noexcept
    : data_(new std::decay_t<T>(std::forward<T>(other))),
      ops_(&obtain_any_ops<T>()) {}

template <typename T>
any& any::operator=(T&& other) {
    if (data_ != nullptr) {
        ops_->destroy(data_);
    }
    data_ = new std::decay_t<T>(std::forward<T>(other));
    ops_ = &obtain_any_ops<T>();
    return *this;
}

template <typename T>
T& any::cast() {
    if (ops_ == nullptr) {
        throw std::runtime_error("No data stored in any");
    }
    return *static_cast<T*>(data_);
}
}
