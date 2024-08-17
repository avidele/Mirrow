#pragma once
#include "dynamic/type_info.hpp"
#include <cstdint>
#include <string_view>
#include <vector>
#include "register.hpp"
#include <iostream>

namespace mirror::dynamic {

class enum_info;
class enum_item final{
    public:
    template<typename T>
    enum_item(std::string_view name, T value, const enum_info* enum_info)
        : name_(name), value_(static_cast<uint32_t>(value)), enum_info_(enum_info) {}

    [[nodiscard]] std::string_view name() const noexcept { return name_; }
    [[nodiscard]] uint32_t value() const noexcept { return value_; }
    [[nodiscard]] const enum_info* info() const noexcept { return enum_info_; }
    private:
    std::string_view name_;
    uint32_t    value_;
    const class enum_info* enum_info_;
};

class enum_info: public type_info{
    public:
    enum_info(std::string_view name, type_id id, value_category category)
        : type_info(name, id, category) {}

    // static uint32_t get_value(const Object& obj){
    //     if(obj.get_type_info()->category_ != value_category::Enum){
    //         throw std::runtime_error("Invalid cast to enum");
    //     }
    //     return *(uint32_t*)obj.GetData();
    // }

    // static void set_value(Object& obj, uint32_t value)
    // {   if(obj.get_type_info()->category_ != value_category::Enum){
    //         throw std::runtime_error("Invalid cast to enum");
    //     }
    //     *(uint32_t*)obj.GetData() = value;
    // }

    enum_info() : type_info("", 0, value_category::Enum) {}
    [[nodiscard]] const enum_item* find_item(std::string_view name) const;
    [[nodiscard]] const enum_item* find_item(uint32_t value) const;
    [[nodiscard]] std::size_t item_count() const;
    [[nodiscard]] const enum_item* item_at(std::size_t index) const;
    void add_item(std::string_view name, uint32_t value);

    private:
    std::vector<enum_item> items_;
};

template<typename T>
class enum_factory{
    public:
    static_assert(std::is_enum_v<T>, "T must be an enum type");
    static enum_factory& instance(){
        static enum_factory factory;
        static bool initialized = false;
        if(!initialized){
            type_dictionary::instance().add(&factory.enum_info_);
            initialized = true;
        }
        return factory;
    }

    auto& regist(const std::string_view& name) noexcept{
        enum_info_.name_ = name;
        if(!type_dictionary::instance().find(name)){
            type_dictionary::instance().add(&enum_info_);
        }else{
            std::cout << "enum type " << name << " already exists" << std::endl;
        }
        return *this;
    }

    template<typename U>
    auto& add_item(const std::string_view& name, U value) noexcept{
        enum_info_.add_item(name, static_cast<uint32_t>(value));
        return *this;
    }
    
    auto& info() noexcept{
        return enum_info_;
    }

    bool has_registered() const noexcept{
        return type_dictionary::instance().find(enum_info_.name());
    }
    
    private:

    class enum_info enum_info_;
};
}  // namespace mirror::dynamic