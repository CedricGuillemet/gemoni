#add_subdirectory(platforms)

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