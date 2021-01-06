#
# Test
#

include(FetchContent)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG master
    GIT_SHALLOW 1
)

FetchContent_MakeAvailable(googletest)

add_library(GTest::Main ALIAS gtest_main)

target_compile_features(gtest PUBLIC cxx_std_17)
target_compile_features(gtest_main PUBLIC cxx_std_17)
target_compile_features(gmock PUBLIC cxx_std_17)
target_compile_features(gmock_main PUBLIC cxx_std_17)

add_executable(tests tests.cpp archetype_tests.cpp storage_tests.cpp entity_manager_tests.cpp)
target_link_libraries(tests PRIVATE ECS GTest::Main Threads::Threads)
add_test(NAME tests COMMAND tests)

add_executable(benchmarks benchmark.cpp)
target_link_libraries(benchmarks PRIVATE ECS)
add_test(NAME benchmarks COMMAND benchmarks)