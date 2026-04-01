cmake_minimum_required(VERSION 3.13)
project(eset_speedtest)
set(CMAKE_CXX_STANDARD 17)
add_executable(code src/main.cpp)
target_compile_options(code PRIVATE -O2 -pipe)

