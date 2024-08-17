#include "iostream"
#include <string_view>

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

}