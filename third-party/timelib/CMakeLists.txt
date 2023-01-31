message(STATUS "Building timelib")
include(ExternalProject)
include(HPHPFunctions)

find_program(RE2C_EXECUTABLE NAMES re2c REQUIRED)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  TIMELIB_DOWNLOAD_ARGS
  SOURCE_URL "https://github.com/derickr/timelib/archive/refs/tags/2021.07.tar.gz"
  SOURCE_HASH "SHA512=8694c1d3420994ea3d94c7f044e467e29f5f707df9d1afe4c2c44233e208eed1b8f3025d21c721695ae741bad9b49f8390d67f818ff7e5fcf79a84a62ed37a0a"
  # Look for timelib-YYYY.MM.tar.gz in download cache, not just YYYY.MM.tar.gz
  FILENAME_PREFIX "timelib-"
)

if(USE_BUNDLED_TZDATA)
  set(HAVE_SYSTEM_TZDATA OFF)
else()
  set(HAVE_SYSTEM_TZDATA ON)
endif()

ExternalProject_Add(
  bundled_timelib
  ${TIMELIB_DOWNLOAD_ARGS}
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/timelib-CMakeLists.txt"
    "<SOURCE_DIR>/CMakeLists.txt"
    &&
    "${CMAKE_CURRENT_SOURCE_DIR}/../apply-quilt-patches.sh"
    "${CMAKE_CURRENT_SOURCE_DIR}/patches"
  CMAKE_ARGS
    "-DRE2C_EXECUTABLE=${RE2C_EXECUTABLE}"
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}

    -DHAVE_SYSTEM_TZDATA=${HAVE_SYSTEM_TZDATA}
    -DHAVE_SYSTEM_TZDATA_PREFIX=${HAVE_SYSTEM_TZDATA_PREFIX}
)

ExternalProject_Get_Property(bundled_timelib INSTALL_DIR)

add_library(timelib INTERFACE)
add_dependencies(timelib bundled_timelib)
target_include_directories(timelib INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(timelib INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}timelib${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
