# This must be done after the 'project' command
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: cmake build type Release.")
endif()
