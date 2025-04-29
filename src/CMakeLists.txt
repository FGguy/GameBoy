add_library(hardware temp.cpp) # add new files here
target_include_directories(hardware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})