cmake_minimum_required(VERSION 3.10)

project(GameBoy)

find_package(SDL2 CONFIG REQUIRED)

add_executable(GameBoy main.cpp)

# Link SDL2
target_link_libraries(GameBoy
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )