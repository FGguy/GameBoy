find_package(SDL2 CONFIG REQUIRED)

add_library(hardware GBPlatform.cpp GBBus.cpp GBCpu.cpp GBPpu.cpp) # add new files here
target_include_directories(hardware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(hardware
        PUBLIC
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )