if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Preventing in-source building. Please run cmake in a seperate build directory.")
endif()