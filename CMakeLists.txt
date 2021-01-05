#
# ECS
#

cmake_minimum_required(VERSION 3.19)

#
# Project Configuration
#

project(
  ECS
  VERSION 0.1.0
  DESCRIPTION "Simple fast entity-component system (ECS)"
  HOMEPAGE_URL "https://github.com/MathieuDonofrio/ecs"
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
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3>
  )
endif()

#
# Target
#

add_library(ECS INTERFACE)
target_include_directories(ECS INTERFACE ${ECS_SOURCE_DIR}/src)
target_compile_features(ECS INTERFACE cxx_std_17)

#
# Tests
#

option(ECS_BUILD_TESTING "Enable building with tests (includes benchmark)" OFF)

if(ECS_BUILD_TESTING)
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif()