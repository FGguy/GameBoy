cmake_minimum_required(VERSION 3.10)

project(GameBoy)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(tests)

add_executable(GameBoy main.cpp)

target_link_libraries(GameBoy
        PRIVATE
        hardware
    )