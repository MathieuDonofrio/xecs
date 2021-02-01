#
# Bench
#

cmake_minimum_required(VERSION 3.15)

add_executable(benchmarks benchmark.cpp)

target_link_libraries(benchmarks PRIVATE XECS)
