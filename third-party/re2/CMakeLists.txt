add_library(re2 INTERFACE)

find_package(RE2)
if(RE2_LIBRARY AND NOT CLANG_FORCE_LIBCPP)
  target_link_libraries(re2 INTERFACE ${RE2_LIBRARY})
  target_include_directories(re2 INTERFACE ${RE2_INCLUDE_DIR})
  return() # from File
endif()

include(ExternalProject)
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  RE2_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/google/re2/archive/refs/tags/2021-11-01.tar.gz"
  SOURCE_HASH
  "SHA256=8c45f7fba029ab41f2a7e6545058d9eec94eef97ce70df58e92d85cfc08b4669"
  FILENAME_PREFIX "re2-"
)

ExternalProject_Add(
  bundled_re2
  ${RE2_SOURCE_ARGS}
  EXCLUDE_FROM_ALL
  CMAKE_ARGS
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}

    -DBUILD_TESTING=OFF
    -DRE2_BUILD_TESTING=OFF
)
ExternalProject_Get_property(bundled_re2 INSTALL_DIR)

add_dependencies(re2 bundled_re2)
target_include_directories(re2 INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(
  re2
  INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}re2${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
