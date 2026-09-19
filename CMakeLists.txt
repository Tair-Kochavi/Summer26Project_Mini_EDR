cmake_minimum_required(VERSION 4.2)
project(mini_edr)

set(CMAKE_CXX_STANDARD 17)

add_executable(mini_edr main.cpp)
add_executable(test1 tests/week1_basic_syscalls/test1.cpp)
add_executable(test2 tests/week1_basic_syscalls/test2.cpp)
add_executable(test3 tests/week1_basic_syscalls/test3.cpp)
add_executable(test4 tests/week1_basic_syscalls/test4.cpp)
