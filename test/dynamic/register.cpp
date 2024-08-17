#include "dynamic/register.hpp"
#include "dynamic/enum_type.hpp"

using namespace mirror::dynamic;

enum class MyEnum{
    A = 1,
    B = 2,
    C = 3,
};

int main()
{
    auto& inst = enum_factory<MyEnum>::instance();
    inst.regist("MyEnum")
    .add_item("A", 1)
    .add_item("B", 2)
    .add_item("C", 3);

    auto& enum_info = inst.info();
    auto& type_dict = type_dictionary::instance();
    auto* info = type_dict.find("MyEnum");
    
    std::cout << (info == &enum_info) << std::endl;
    std::cout << enum_info.find_item("A")->value() << std::endl;
    return 0;
}
