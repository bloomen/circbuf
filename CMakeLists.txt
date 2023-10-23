cmake_minimum_required(VERSION 3.0)
project(circbuf)

option(circbuf_build_tests "Whether to build the circbuf tests" ON)
option(circbuf_enable_asan "Build circbuf tests with address sanitizer." ON)
set(circbuf_clang_format clang-format CACHE STRING "Clang format binary")

install(FILES include/circbuf.h DESTINATION include)

if (circbuf_build_tests)

    enable_testing()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if (MSVC)
        add_compile_options(/W4 /bigobj /EHsc /wd4503 /wd4996 /wd4702 /wd4100)
        if(${MSVC_VERSION} GREATER_EQUAL 1929)
            if(circbuf_enable_asan)
                add_compile_options(/fsanitize=address)
            endif()
        endif()
    else()
        add_compile_options(-Wpedantic -Wall -Wextra -Wconversion -Wno-parentheses)
        if(circbuf_enable_asan)
            add_compile_options(-fsanitize=address)
            add_link_options(-fsanitize=address)
        endif()
    endif()

    include_directories(include)

    set(circbuf_TEST_SOURCES
        test/catch_amalgamated.hpp
        test/catch_amalgamated.cpp
        test/test.cpp)

    add_executable(circbuf_test ${circbuf_TEST_SOURCES})
    add_test(circbuf_test circbuf_test)
endif()

set(circbuf_source_files
    ${PROJECT_SOURCE_DIR}/include/circbuf.h
    ${PROJECT_SOURCE_DIR}/test/test.cpp)

add_custom_target(
    circbuf_format
    COMMAND ${circbuf_clang_format}
    -style=file
    -i
    ${circbuf_source_files}
)
