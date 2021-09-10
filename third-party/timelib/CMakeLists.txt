message(STATUS "Building timelib")
include(ExternalProject)
include(HPHPFunctions)

find_program(RE2C_EXECUTABLE NAMES re2c REQUIRED)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  TIMELIB_DOWNLOAD_ARGS
  SOURCE_URL "https://github.com/derickr/timelib/archive/refs/tags/2021.07.tar.gz"
  SOURCE_HASH "SHA512=7bc56d20360937af10f63960e443cc8bd4d24c5369f697241e54da21465d4512bd16cfa6f0efcf2b847cc19781e1cecf93c9e19a1efa4f1a7012c9fa442eeabe"
  # Look for timelib-YYYY.MM.tar.gz in download cache, not just YYYY.MM.tar.gz
  FILENAME_PREFIX "timelib-"
)

ExternalProject_Add(
  bundled_timelib
  ${TIMELIB_DOWNLOAD_ARGS}
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/timelib-CMakeLists.txt"
    "<SOURCE_DIR>/CMakeLists.txt"
  CMAKE_ARGS
    "-DRE2C_EXECUTABLE=${RE2C_EXECUTABLE}"
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
)

ExternalProject_Get_Property(bundled_timelib INSTALL_DIR)

add_library(timelib INTERFACE)
add_dependencies(timelib bundled_timelib)
target_include_directories(timelib INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(timelib INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}timelib${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
