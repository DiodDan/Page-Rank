cmake_minimum_required(VERSION 3.10)
project(Page-Rank)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    link_libraries(OpenMP::OpenMP_CXX)
endif()

add_executable(Page-Rank cpp/main.cpp)