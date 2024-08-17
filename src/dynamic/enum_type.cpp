#include "dynamic/enum_type.hpp"
using namespace mirror::dynamic;

const enum_item* enum_info::find_item(std::string_view name) const{
    for (auto& item : items_) {
        if (item.name() == name) {
            return &item;
        }
    }
    return nullptr;
}

const enum_item* enum_info::find_item(uint32_t value) const{
    for (auto& item : items_) {
        if (item.value() == value) {
            return &item;
        }
    }
    return nullptr;
}

std::size_t enum_info::item_count() const{
    return items_.size();
}

const enum_item* enum_info::item_at(std::size_t index) const{
    return &items_[index];
}

void enum_info::add_item(std::string_view name, uint32_t value){
    items_.emplace_back(name, value, this);
}

