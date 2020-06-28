
file(GLOB SRC_FILES
    *.h
    *.cpp
)

add_library(stb ${SRC_FILES})
target_include_directories(stb PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_property(TARGET stb PROPERTY FOLDER Dependencies)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})