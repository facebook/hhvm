message(STATUS "Building timelib")
include(ExternalProject)

set(TIMELIB_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/timelib-prefix")

ExternalProject_Add(
  timelibBuild
  GIT_REPOSITORY https://github.com/derickr/timelib
  GIT_TAG 2021.07
  PATCH_COMMAND
    patch -p1 < "${CMAKE_CURRENT_SOURCE_DIR}/0001-add-CMakeLists.txt.patch"
  PREFIX "${TIMELIB_PREFIX}"
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${TIMELIB_PREFIX}
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
)

add_library(timelib INTERFACE)
add_dependencies(timelib timelibBuild)
target_include_directories(timelib INTERFACE "${TIMELIB_PREFIX}/include")
target_link_libraries(timelib INTERFACE
  "${TIMELIB_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}timelib${CMAKE_STATIC_LIBRARY_SUFFIX}"
)