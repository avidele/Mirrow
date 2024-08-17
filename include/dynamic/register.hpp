#pragma once
#include "dynamic/type_info.hpp"
#include <unordered_map>
namespace mirror::dynamic {
class type_dictionary {
public:
    static auto& instance() {
        static type_dictionary instance;
        return instance;
    }

    void add(const type_info* type) {
        if(type && !types_.contains(type->name())) {
            types_[type->name()] = type;
        }
    }

    const type_info* find(std::string_view name) const {
        auto it = types_.find(name);
        return it != types_.end() ? it->second : nullptr;
    }

    auto& typeinfos() const { return types_; }

    private:
    std::unordered_map<std::string_view, const type_info*> types_;
    type_dictionary() = default;
};
}  // namespace mirror::dynamic