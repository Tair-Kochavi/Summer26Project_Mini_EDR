cmake_minimum_required(VERSION 4.2)
project(mini_edr)

set(CMAKE_CXX_STANDARD 17)

add_executable(mini_edr main.cpp
        syscall_format.cpp
        syscall_format.h
        blacklist.cpp
        blacklist.h
        nw_blacklist.cpp
        nw_blacklist.h)
add_executable(test1 tests/week1_basic_syscalls/test1.cpp)
add_executable(test2 tests/week1_basic_syscalls/test2.cpp)
add_executable(test3 tests/week1_basic_syscalls/test3.cpp)
add_executable(test4 tests/week1_basic_syscalls/test4.cpp)


add_executable(test_process_lifecycle tests/additional_tests/test_process_lifecycle.cpp)

# Test 2 requires the pthread library to use clone()
add_executable(test_threads_interleave tests/additional_tests/test_threads_interleave.cpp)
target_link_libraries(test_threads_interleave pthread)

add_executable(test_file_edge_cases tests/additional_tests/test_file_edge_cases.cpp)
add_executable(test_network_edge_cases tests/additional_tests/test_network_edge_cases.cpp)
add_executable(test_shellcode_wx tests/additional_tests/test_shellcode_wx.cpp)
