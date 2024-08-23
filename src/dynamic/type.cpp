#include "dynamic/type.hpp"
#include <stdexcept>
using namespace mirror::dynamic;

type::~type() = default;

const std::string_view& type::get_name() const noexcept { return name_; }

const std::string_view& type::get_field(std::string_view key) const {
    if(!has_field(key)){
        throw std::runtime_error("Field not found");
    }
    return fields_.at(key);
}

std::string type::fetch_all_fields() const {
    std::string result;
    for(const auto& [key, value]: fields_){
        result += std::string(key);
        result += ": ";
        result += value;
        result += "\n";
    }
    return result;
}

bool type::has_field(std::string_view key) const {
    return fields_.contains(key);
}