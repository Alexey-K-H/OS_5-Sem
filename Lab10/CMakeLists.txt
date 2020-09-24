cmake_minimum_required(VERSION 3.17)
project(Lab10 C)

set(CMAKE_C_STANDARD 99)

add_executable(Lab10 main.c)
target_link_libraries(Lab10 pthread)