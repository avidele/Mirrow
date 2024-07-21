#include "util/variable_tarits.hpp"

using namespace mirror::util;

char* name{};

struct TestClass{
    int Score;
    static float StaticScore;
    const float ConstScore;
};

int main()
{
    using ptr_traits = variable_pointer_traits<&name>;
    static_assert(!ptr_traits ::is_member);
    static_assert(std::is_same_v<ptr_traits ::type, char*>);

    using static_class_traits = variable_pointer_traits<&TestClass::StaticScore>;
    static_assert(!static_class_traits ::is_member);
    static_assert(std::is_same_v<static_class_traits::pointer, float*>);

    using const_class_traits = variable_pointer_traits<&TestClass::ConstScore>;
    static_assert(const_class_traits::is_member);
    static_assert(std::is_same_v<const_class_traits::pointer, const float TestClass::*>);
    return 0;
}