#include "dynamic/register.hpp"
#include "dynamic/type.hpp"

using namespace mirror::dynamic;
int v0 = 5;

enum class MyEnum {
    A = 1,
    B = 2,
    C = 3,
};

int F0(const int a, const int b) {
    return a + b;
}

int& F1() {
    return v0;
}

void F2(int& outValue) {
    outValue = 1;
}

void F4(int outValue) {
    outValue = 1;
}

void F3(int* outValue) {
    *outValue = 4;
}

class C0{
public:
    static int v0;
    int v1;
    static int F4(const int a, const int b) {
        return a + b;
    }
    int F0(const int a, const int b) {
        return a + b;
    }
    int& F1() {
        return v0;
    }
    void F2(int& outValue) {
        outValue = 1;
    }
    void F3(int* outValue) {
        *outValue = 4;
    }
};

int C0::v0 = 115;


int main() {
    // registry::instance()
    //     .global()
    //     .variable<&v0>("v0")
    //     .meta_data("name", "v0")
    //     .function<&F0>("F0")
    //     .function<&F1>("F1")
    //     .function<&F2>("F2");
    // const auto& registry_global_context = global_context::get_instance();
    // {
    //     const auto& v0 = registry_global_context.get_variable("v0");
    //     auto value = v0.get_value();
    //     std::cout << value.cast<int>() << std::endl;
    //     // value.cast<int&>() = 2;
    //     // std::cout << v0.get_value().cast<int>() << std::endl;
    //     v0.set_value(4);
    //     std::cout << v0.get_value().cast<int>() << std::endl;
    //     any any_value = 15;
    //     v0.set_value(&any_value);
    //     std::cout << v0.get_value().cast<int>() << std::endl;
    // }
    // {
    //     const auto& F0 = registry_global_context.get_function("F0");
    //     auto result = F0.call(1, 2);
    //     std::cout << "F0 call result: " << result.cast<int>() << std::endl;

    //     const auto& F1 = registry_global_context.get_function("F1");
    //     auto v0 = F1.call().cast<int>();
    //     std::cout << "F1 call result: " << v0 << std::endl;

    //     const auto& F2_f = registry_global_context.get_function("F2");
    //     int value = 0;
    //     F2_f.call(value);
    //     std::cout << "F2 call result: " << value << std::endl;

    //     F4(std::ref(value));
    //     std::cout << "F4 call result: " << value << std::endl;
    // }

    registry::instance().Class<C0>("C0")
    .static_variable<&C0::v0>("v0")
    .static_function<&C0::F4>("F4")
    .member_variable<&C0::v1>("v1")
    .member_function<&C0::F0>("F0");

    const auto& clazz = class_type::get("C0");

    std::cout << C0::v0 << std::endl;
    return 0;
}
