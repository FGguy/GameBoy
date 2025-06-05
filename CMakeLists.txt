cmake_minimum_required(VERSION 3.10)

project(GameBoy)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(tests)

add_executable(GameBoy main.cpp)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    # For GCC and Clang
    target_compile_options(Gameboy PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # For MSVC
    target_compile_options(GameBoy PRIVATE
        /W4           # Warning level 4 (max useful)
        /permissive-  # Standard-conforming mode
        /w14242       # 'identifier' used but never initialized
        /w14287       # Unsigned/negative mismatch
        /w14296       # Expression is always true/false
        /w14311       # Pointer truncation
        /WX           # Treat warnings as errors
    )
endif()

target_link_libraries(GameBoy
        PRIVATE
        hardware
    )