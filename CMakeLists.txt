cmake_minimum_required(VERSION 3.10)

project(GameBoy)

find_package(SDL2 CONFIG REQUIRED)

add_subdirectory(src)

add_executable(GameBoy main.cpp)

# Link SDL2
target_link_libraries(GameBoy
        PRIVATE
        hardware
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> #targets for SDL2, to be moved to files that actually use it later
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )