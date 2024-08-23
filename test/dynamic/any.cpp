#include "iostream"
#include <string_view>
#include <type_traits>
#include "dynamic/any.hpp"

using namespace mirror::dynamic;
enum class MyEnum {
    STRING,
    INTEGER
};

class A{
    int a;
    void B();
    public:
    explicit A(int a): a(a){}
};

struct C{
    std::string name;
    int age;
    public:
    void say(std::string_view message){
        std::cout << name << " says " << message << std::endl;}
};

int main()
{
    mirror::dynamic::any a = 5;
    mirror::dynamic::any b = std::string("Hello");
    mirror::dynamic::any c = MyEnum::STRING;
    mirror::dynamic::any d = A(5);
    mirror::dynamic::any e = C{"John", 25};

    any e2 = e;
    static_assert(std::is_same_v<decltype(a.cast<int>()), int&>);
    mirror::dynamic::any f = std::move(a);
    mirror::dynamic::any g = std::move(b);
    mirror::dynamic::any h = std::move(c);
    mirror::dynamic::any i = std::move(d);
    mirror::dynamic::any j = std::move(e);

    return 0;
}