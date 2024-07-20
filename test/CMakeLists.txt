macro(AddTest name)
    add_executable(${name} ${name}.cpp)
    target_compile_features(${name} PUBLIC cxx_std_17)
    target_link_libraries(${name} PRIVATE mirrow)
    target_include_directories(${name} PRIVATE ../)
    add_test(NAME ${name}
             COMMAND $<TARGET_FILE:${name}>)
endmacro(AddTest)

add_subdirectory(utils)