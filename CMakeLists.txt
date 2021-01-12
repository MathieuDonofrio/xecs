#
# ECS
#

cmake_minimum_required(VERSION 3.14)

#
# Project Configuration
#

project(
  XECS
  VERSION 1.0.0
  DESCRIPTION "Fast entity-component-system (ECS) with compile-time archetypes"
  HOMEPAGE_URL "https://github.com/MathieuDonofrio/xecs"
  LANGUAGES CXX
)

#
# Compiler Options
#

if(MSVC)
  add_compile_options(
    /W4
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Release>:/O2>
  )
else()
  add_compile_options(
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-O3>
  )
endif()

#
# Target
#

add_library(XECS INTERFACE)
target_include_directories(XECS INTERFACE ${XECS_SOURCE_DIR}/src)
target_compile_features(XECS INTERFACE cxx_std_17)

#
# Tests
#

option(XECS_BUILD_TESTING "Enable building with tests" OFF)

if(XECS_BUILD_TESTING)
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif()

#
# Benchmarks
#

option(XECS_BUILD_BENCHMARKING "Enable building with benchmarks" OFF)

if(XECS_BUILD_BENCHMARKING)
  add_subdirectory(bench)
endif()