#pragma once

namespace mirror{
    namespace util{
        namespace detail{
            
            template <typename T>
            struct variable_type {
                using type = T;
            };

            template <typename Class, typename T>
            struct variable_type<T Class::*> {
                using type = T;
            };

            template <typename T>
            auto variable_pointer_to_type(long, T*) -> T;

            template <typename Class, typename T>
            auto variable_pointer_to_type(char, T Class::*) -> T;
        }

        template <typename T>
        using variable_type_t = typename detail::variable_type<T>::type;

        template <auto V>
        using variable_pointer_to_type_t = decltype(detail::variable_pointer_to_type(0, V));
        
        
    }
}