
file(GLOB SRC_FILES
    *.h
    *.cpp
    *.mm
)

add_library(imgui ${SRC_FILES})
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(imgui bgfx)

set_property(TARGET imgui PROPERTY FOLDER Dependencies)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})