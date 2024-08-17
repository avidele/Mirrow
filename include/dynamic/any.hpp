#include <algorithm>
#include <concepts>
#include <iostream>

namespace mirror::dynamic{
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
any_ops& any_ops_for() {
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

private:
    void* data_ = nullptr;

};
}
