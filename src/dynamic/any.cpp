#include "dynamic/any.hpp"
using namespace mirror::dynamic;

any::any(const any& other) {
    if (other.data_ == nullptr) {
        data_ = nullptr;
        ops_ = nullptr;
    } else {
        data_ = ops_->copy(other.data_);
        ops_ = other.ops_;
    }
}

any::any(any&& other) noexcept
{
    data_ = other.data_;
    ops_ = other.ops_;
    other.data_ = nullptr;
    other.ops_ = nullptr;
}

any::~any() {
    if (data_ != nullptr) {
        ops_->destroy(data_);
    }
}

any& any::operator=(const any& other) {
    if (this == &other) {
        return *this;
    }
    if (data_ != nullptr) {
        ops_->destroy(data_);
    }
    if (other.data_ == nullptr) {
        data_ = nullptr;
        ops_ = nullptr;
    } else {
        data_ = ops_->copy(other.data_);
        ops_ = other.ops_;
    }
    return *this;
}

any& any::operator=(any&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (data_ != nullptr) {
        ops_->destroy(data_);
    }
    data_ = other.data_;
    ops_ = other.ops_;
    other.data_ = nullptr;
    other.ops_ = nullptr;
    return *this;
}

