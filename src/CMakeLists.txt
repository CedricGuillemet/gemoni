
if(WIN32)
    set(SRC_FILES
        "platforms/main_win32.cpp")
elseif(APPLE)
    set(SRC_FILES
        "platforms/main.m"
        "platforms/AppDelegate.h"
        "platforms/AppDelegate.m"
        "platforms/ViewController.h"
        "platforms/ViewController.mm"
        "platforms/main_osx.mm"
        "platforms/main_osx.h")
elseif(__linux__)
    set(SRC_FILES
        "platforms/main_x11.cpp")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(APPLE)
    set(PLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/platforms/Info.plist")
    set(STORYBOARD "${CMAKE_CURRENT_LIST_DIR}/platforms/Base.lproj/Main.storyboard")
    set(RESOURCE_FILES ${STORYBOARD})
endif()

add_subdirectory(ui)
add_executable(Gemoni ${SRC_FILES} ${RESOURCE_FILES})

if(APPLE)
    set_target_properties(Gemoni PROPERTIES
            MACOSX_BUNDLE true
            MACOSX_BUNDLE_INFO_PLIST "${PLIST_FILE}"
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
            RESOURCE "${RESOURCE_FILES}"
            FOLDER "Gemoni")
endif()

target_link_libraries(Gemoni imgui bgfx bx bimg imwidgets stb ui)

set_target_properties(Gemoni PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(Gemoni PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(Gemoni PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(Gemoni PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(Gemoni PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")
set_target_properties(Gemoni PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")