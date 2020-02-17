cmake_minimum_required(VERSION 3.10)
project(thr_vs_prc CXX)

set(CMAKE_CXX_STANDARD 14)
option(USE_SYSV_SHM "use SYSV Shared Memory instead of mmap" OFF)

add_subdirectory(cos_log)
add_subdirectory(src)