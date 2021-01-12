#
# Bench
#

add_executable(benchmarks benchmark.cpp)
target_link_libraries(benchmarks PRIVATE XECS)
add_test(NAME benchmarks COMMAND benchmarks)