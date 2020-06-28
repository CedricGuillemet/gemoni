
if(WIN32)
  
set(SRC_FILES
    "platforms/main_win32.cpp")
    
elseif(APPLE)

set(SRC_FILES
    "platforms/main.m"
    "platforms/AppDelegate.h"
    "platforms/AppDelegate.m"
    "platforms/ViewController.h"
    "platforms/ViewController.m"
    "platforms/main_osx.mm"
    "platforms/main_osx.h")

else()

endif()

add_subdirectory(ui)
add_executable(Gemoni ${SRC_FILES})
target_link_libraries(Gemoni imgui bgfx bx bimg imwidgets stb ui)

set_target_properties(Gemoni PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(Gemoni PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(Gemoni PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(Gemoni PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(Gemoni PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")
set_target_properties(Gemoni PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")