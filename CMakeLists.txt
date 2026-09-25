cmake_minimum_required(VERSION 3.15)
project(sdfl_deflate VERSION 1.0 LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
 
add_compile_options(-Wall -Wextra)
 
file(GLOB SOURCES CONFIGURED_DEPENDS "src/*.cpp")
 
add_executable(deflate ${SOURCES})
target_include_directories(deflate PRIVATE include)
 