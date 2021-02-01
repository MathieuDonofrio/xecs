#
# Test
#

cmake_minimum_required(VERSION 3.15)

include(FetchContent)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.10.0
)

FetchContent_MakeAvailable(googletest)

add_library(GTest::Main ALIAS gtest_main)

target_compile_features(gtest PUBLIC cxx_std_17)
target_compile_features(gtest_main PUBLIC cxx_std_17)

add_executable(tests tests.cpp archetype_tests.cpp storage_tests.cpp entity_manager_tests.cpp registry_tests.cpp)
target_link_libraries(tests PRIVATE XECS GTest::Main Threads::Threads)
add_test(NAME tests COMMAND tests)