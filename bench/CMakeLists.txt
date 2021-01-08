#
# Bench
#

add_executable(benchmarks benchmark.cpp)
target_link_libraries(benchmarks PRIVATE ECS)
add_test(NAME benchmarks COMMAND benchmarks)