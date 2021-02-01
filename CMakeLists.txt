#
# XECS
#

cmake_minimum_required(VERSION 3.15)

#
# Project Configuration
#

include(cmake/PreventInSourceBuilds.cmake)

project(
  XECS
  VERSION 1.0.0
  DESCRIPTION "Fast entity-component-system (ECS) with compile-time archetypes"
  HOMEPAGE_URL "https://github.com/MathieuDonofrio/xecs"
  LANGUAGES CXX
)

include(cmake/CompilerOptions.cmake)

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

#
# Documentation
#

option(XECS_BUILD_DOCUMENTATION "Enable doxygen doc builds of source" OFF)

if(XECS_BUILD_BENCHMARKING)
  set(DOXYGEN_CALLER_GRAPH YES)
  set(DOXYGEN_CALL_GRAPH YES)
  set(DOXYGEN_EXTRACT_ALL YES)
  find_package(Doxygen REQUIRED dot)
  doxygen_add_docs(doxygen-docs ${XECS_SOURCE_DIR}/src)
endif()