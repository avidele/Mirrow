#include "dynamic/register.hpp"
#include "dynamic/any.hpp"
#include "dynamic/type.hpp"
#include "gtest/gtest.h"
#include <string_view>

using namespace mirror::dynamic;
int v0 = 5;

enum class MyEnum {
    A = 1,
    B = 2,
    C = 3,
    D = 114,
    E = 514,
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

class C0 {
public:
    static int v0;
    int v1;

    static int F4(const int a, const int b) { return a + b; }

    int F0(const int a, const int b) { return a + b; }

    int& F1() { return v0; }

    void F2(int& outValue) { outValue = 1; }

    void F3(int* outValue) { *outValue = 4; }
};

class C1 {
public:
    explicit C1(const int inV0) : v0(inV0) {}

    int GetV0() { return ++v0; }

    void SetV0(int value) { v0 = value; }

private:
    int v0;
};

struct C2 {
    C2(const int inA, const int inB) : a(inA), b(inB) {}

    int a;
    int b;
    std::string_view c;
};

struct C3 : C2 {
    C3(const int inA, const int inB, const int inC) : C2(inA, inB), c(inC) {}

    int c;
};

int C0::v0 = 115;

TEST(register, global_test) {
    registry::instance()
        .global()
        .variable<&v0>("v0")
        .meta_data("name", "v0")
        .function<&F0>("F0")
        .function<&F1>("F1")
        .function<&F2>("F2");
    const auto& registry_global_context = global_context::get_instance();
    {
        const auto& v0 = registry_global_context.get_variable("v0");
        auto value = v0.get_value();
        ASSERT_EQ(value.cast<int>(), 5);
        v0.set_value(4);
        ASSERT_EQ(v0.get_value().cast<int>(), 4);
        any any_value = 15;
        v0.set_value(&any_value);
        ASSERT_EQ(v0.get_value().cast<int>(), 15);
    }
    {
        const auto& F0 = registry_global_context.get_function("F0");
        auto result = F0.call(1, 2);
        ASSERT_EQ(result.cast<int>(), 3);
        const auto& F1 = registry_global_context.get_function("F1");
        auto v0 = F1.call().cast<int>();
        ASSERT_EQ(v0, 15);
        const auto& F2_f = registry_global_context.get_function("F2");
        int value = 0;
        F2_f.call(value);
        ASSERT_EQ(value, 0);
        F4(std::ref(value));
        ASSERT_EQ(value, 0);
    }
}

TEST(register, class_test)
{
    registry::instance()
        .Class<C0>("C0")
        .static_variable<&C0::v0>("v0")
        .static_function<&C0::F4>("F4")
        .member_variable<&C0::v1>("v1")
        .member_function<&C0::F0>("F0");

    const auto& clazz = class_type::get("C0");
    const auto& v0 = clazz.get_static_variable("v0");
    v0.set_value(5);

    const auto& F4 = clazz.get_static_function("F4");
    auto result = F4.call(1, 2);
    ASSERT_EQ(result.cast<int>(), 3);

    registry::instance()
        .Class<C1>("C1")
        .constructor<int>("Construct0")
        .member_function<&C1::GetV0>("GetV0")
        .member_function<&C1::SetV0>("SetV0");

    const auto& clazz1 = class_type::get("C1");
    const auto& constructor = clazz1.get_constructor("Construct0");
    const auto& setter = clazz1.get_member_function("SetV0");
    const auto& getter = clazz1.get_member_function("GetV0");

    auto object = constructor.construct_on_stack(1);
    ASSERT_EQ(getter.call(object.cast<C1&>()).cast<int>(), 2);
    setter.call(object.cast<C1&>(), 2);
    ASSERT_EQ(getter.call(object.cast<C1&>()).cast<int>(), 2);

    // Construct On Stack will recreate the object
    setter.call(object.cast<C1&>(), 3);
    ASSERT_EQ(getter.call(object.cast<C1&>()).cast<int>(), 2);

     registry::instance()
        .Class<C2>("C2")
        .constructor<int, int>("Construct0")
        .member_variable<&C2::a>("a")
        .member_variable<&C2::b>("b")
        .member_variable<&C2::c>("c");

    const auto& clazz2 = class_type::get("C2");
    const auto& constructor2 = clazz2.get_constructor("Construct0");
    const auto& a = clazz2.get_member_variable("a");
    const auto& b = clazz2.get_member_variable("b");
    const auto& c = clazz2.get_member_variable("c");
    const auto& destructor = clazz2.get_destructor();

    auto object2 = constructor2.new_object(1, 2, "Hello");
    auto object_ref = any(*object2.cast<C2*>());
    auto value_a = a.get_value(&object_ref);
    auto value_b = b.get_value(&object_ref);
    auto value_c = c.get_value(&object_ref);

    ASSERT_EQ(value_a.cast<int>(), 1);
    ASSERT_EQ(value_b.cast<int>(), 2);
    destructor.call(&object_ref);

    registry::instance()
        .Class<C3, C2>("C3")
        .constructor<int, int, int>("Construct0")
        .member_variable<&C3::c>("c");

    const auto& clazz3 = class_type::get("C3");
    const auto& constructor3 = clazz3.get_constructor("Construct0");
    auto c3 = constructor3.new_object(1, 2, 3).cast<C2*>();
    ASSERT_EQ(c3->a, 1);
    ASSERT_EQ(c3->b, 2);
}

TEST(register, enum_test)
{
    registry::instance()
        .Enum<MyEnum>("MyEnum")
        .item("A", MyEnum::A)
        .item("B", MyEnum::B)
        .item("C", MyEnum::C)
        .item("D", MyEnum::D)
        .item("E", MyEnum::E);

    const auto& my_enum = enum_type::get("MyEnum");
    const auto& item = my_enum.get_item("A");
    const auto& item2 = my_enum.get_item("E");
    ASSERT_EQ(item.get().cast<int>(), 1);
    ASSERT_EQ(item2.get().cast<int>(), 514);
    // std::cout << v0.get_value().cast<int>() << std::endl;

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
