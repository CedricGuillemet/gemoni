
file(GLOB SRC_FILES
    *.h
    *.cpp
)

add_library(imwidgets ${SRC_FILES})
target_include_directories(imwidgets PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(imwidgets imgui)

set_property(TARGET imwidgets PROPERTY FOLDER Dependencies)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})