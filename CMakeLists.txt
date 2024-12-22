cmake_minimum_required(VERSION 3.10)

project(untitled3)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(untitled3
        main.cpp
        pixel.h
        Makson.cpp
        Makson2.cpp
)

target_link_libraries(untitled3 ${OpenCV_LIBS})