if(NOT CMAKE_BUILD_TYPE)
    message("-- No build type specified. Assumed debug.")
    set(CMAKE_BUILD_TYPE Debug)
endif()

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