if (NOT CLANG_FORCE_LIBCPP)
  find_package(gflags CONFIG)

  if (TARGET gflags::gflags)
    message(STATUS "Using system gflags")

    add_library(gflags INTERFACE)
    add_dependencies(gflags gflags::gflags)
    target_link_libraries(gflags INTERFACE gflags::gflags)

    get_target_property(GFLAGS_INCLUDE_DIR gflags::gflags INTERFACE_INCLUDE_DIRECTORIES)

    target_include_directories(gflags INTERFACE ${GFLAGS_INCLUDE_DIR})

    return()
  endif()
endif()

message(STATUS "Using third-party bundled gflags")
include(ExternalProject)
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  GFLAGS_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz"
  SOURCE_HASH
  "SHA256=34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf"
)

set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/gflags-prefix")
ExternalProject_add(
  bundled_gflags
  ${GFLAGS_SOURCE_ARGS}
  CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_INSTALL_INCLUDEDIR=include
      -DCMAKE_INSTALL_LIBDIR=lib
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
)

ExternalProject_get_property(bundled_gflags INSTALL_DIR)

add_library(gflags INTERFACE)
add_dependencies(gflags bundled_gflags)
target_include_directories(gflags INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(gflags INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gflags$<$<CONFIG:Debug>:_debug>${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

set(gflags_DIR "${INSTALL_DIR}/lib/cmake/gflags" PARENT_SCOPE)
