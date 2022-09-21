find_package(fmt 8 CONFIG)
if (fmt_DIR)
  message(STATUS "Using system fmt: ${fmt_DIR}")
else()
  message(STATUS "Using third-party bundled fmt")
  include(ExternalProject)
  include(HPHPFunctions)

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    FMT_SOURCE_ARGS
    SOURCE_URL
    "https://github.com/fmtlib/fmt/releases/download/8.0.0/fmt-8.0.0.zip"
    SOURCE_HASH
    "SHA512=70848739642bebf2820d661e7547208445329325a0d8b5919d85f375d683c0f9bef763bc7e9d7823def15100134fd9a126a9e418223b20bf8d567e5084334778"
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