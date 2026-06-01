find_package(fmt 9 CONFIG)
if (TARGET fmt::fmt AND NOT CLANG_FORCE_LIBCPP)
  message(STATUS "Using system fmt: ${fmt_DIR}")
  add_library(fmt INTERFACE)
  add_dependencies(fmt fmt::fmt)
  get_target_property(FMT_INCLUDE_DIR fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
  target_link_libraries(fmt INTERFACE fmt::fmt)
  target_include_directories(fmt INTERFACE ${FMT_INCLUDE_DIR})
else()
  message(STATUS "Using third-party bundled fmt")
  include(ExternalProject)
  include(HPHPFunctions)

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    FMT_SOURCE_ARGS
    SOURCE_URL
    "https://github.com/fmtlib/fmt/releases/download/12.1.0/fmt-12.1.0.zip"
    SOURCE_HASH
    "SHA512=405645141c93dcde2225976328a4b373e5ca3b0bc1c1759249ce3cee2ec86dc6080bec5d4adafb9d6b5f5501e5f8e8fe8bcdba9107047b54f8ef46a4a8f5971e"
  )

  set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/fmt-prefix")
  ExternalProject_add(
    bundled_fmt
    ${FMT_SOURCE_ARGS}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_INSTALL_INCLUDEDIR=include
      -DCMAKE_INSTALL_LIBDIR=lib
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
      -DFMT_TEST=Off
  )
  cmake_minimum_required(VERSION 2.8.0)
  ExternalProject_get_property(bundled_fmt INSTALL_DIR)

  add_library(fmt INTERFACE)
  add_dependencies(fmt bundled_fmt)
  target_include_directories(fmt INTERFACE "${INSTALL_DIR}/include")
  target_link_libraries(fmt INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fmt$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
  set(fmt_DIR "${INSTALL_DIR}/lib/cmake/fmt" PARENT_SCOPE)
endif()
