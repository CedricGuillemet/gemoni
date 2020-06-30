cmake_minimum_required (VERSION 3.6)
project(Gemoni)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(ext)
add_subdirectory(src)