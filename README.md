<p align="center">
<img src="https://img.shields.io/badge/std-C++20-green">
<img src="https://img.shields.io/badge/License-MIT-blue">
<img src="https://img.shields.io/badge/License-MIT-blue">
</p>

## Preface
This is a reflection library for cpp, which is planned for my future game engine.Now, it has supported global types, class types, and enum types. More types and methods will be supported in the future.

This library has not finished for official use, only for study and discussion.

## Build
Create a build directory.
Generate a build system using any desired generator in CMake.
Build - you can use any IDE if applicable to the generator, but you can also just build straight from CMake.

Example:
```shell
CMake -B build .
CMake --build build 
```

## Example

```cpp
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

```

```cpp
TEST(register, class_test)
{
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
```


