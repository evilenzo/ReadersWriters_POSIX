cmake_minimum_required(VERSION 3.21)

project(ReadersWriters_POSIX)

set(CMAKE_CXX_STANDARD 20)

add_executable(ReadersWriters_POSIX main.cpp)
target_link_libraries(ReadersWriters_POSIX pthread)
