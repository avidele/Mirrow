#include "sre/sre.hpp"
#include <string>
#include <utility>


using namespace mirror;

class Person final {
public:
    static std::string family_name;

    Person(std::string  name, float height)
        : name(std::move(name)), height(height) {}

    void AddChild(const Person& person) { children.push_back(person); }

    [[nodiscard]] std::string Name() const { return name; }

    [[nodiscard]] float Height() const { return height; }

    Person& operator+(const Person& child) {
        AddChild(child);
        return *this;
    }

    std::string name;
    float height;
    std::vector<Person> children;
};

std::string Person::family_name = "Avidel";
int main()
{

    static_assert(mirror::get_name("Test::Person")=="Person");

}