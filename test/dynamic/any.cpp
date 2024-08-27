#include "iostream"
#include <string>
#include <string_view>
#include "dynamic/any.hpp"
#include <any>
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

struct AnyTestStruct3 {
    std::string value;
    bool heapObjectAllocated;
    int* heapObject;

    AnyTestStruct3(std::string inValue, int inHeapObjValue)
        : value(std::move(inValue))
        , heapObjectAllocated(true)
        , heapObject(new int(inHeapObjValue))
    {
    }

    ~AnyTestStruct3()
    {
        if (heapObjectAllocated) {
            delete heapObject;
        }
    }

    AnyTestStruct3(AnyTestStruct3&& inOther) noexcept
        : value(std::move(inOther.value))
        , heapObjectAllocated(inOther.heapObjectAllocated)
        , heapObject(inOther.heapObject)
    {
        inOther.heapObjectAllocated = false;
        inOther.heapObject = nullptr;
    }
};

int main()
{
    // any a = 1;
    // any b = std::string("Hello");
    // // any c = b;
    // any d = C{"John", 25};
    // std::string name = "John";
    // any e = name;

    // const any f = 1;
    // std::cout << f.cast<int>() << std::endl;

    // C c0 = C("John", 25);
    // any g = std::move(c0);
    // std::cout << g.cast<C>().name << std::endl;

    // constexpr auto h = 1;
    // any i = h;
    // std::cout << i.cast<const int>() << std::endl;

    // const any j = AnyTestStruct3("Hello", 5);
    // any k(333);
    // k = 33;
    // std::cout << k.cast<int>() << std::endl;

    int value = 0;
    any l = std::ref(value);
    l.cast<int&>() = 5;
    std::cout << l.cast<int>() << " " << value << std::endl;

    int value2 = 0;
    any m(std::ref(value2));
    m.cast<int&>() = 5;
    float f0 = 1.0f;
    m = std::ref(f0);
    m.cast<float&>() = 2.0f;
    std::cout << m.cast<float&>() << " " << f0 << std::endl;
}