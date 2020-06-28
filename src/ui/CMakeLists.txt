
file(GLOB SRC_FILES
    *.h
    *.cpp
)

add_library(ui ${SRC_FILES})
target_include_directories(ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ui imgui)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})