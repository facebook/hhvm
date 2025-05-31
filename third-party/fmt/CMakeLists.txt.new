find_package(fmt 9 CONFIG)
if (TARGET fmt::fmt AND NOT CLANG_FORCE_LIBCPP)
  message(STATUS "Using system fmt: ${fmt_DIR}")
  add_library(fmt INTERFACE)
  add_dependencies(fmt fmt::fmt)
  target_link_libraries(fmt INTERFACE fmt::fmt)
  get_target_property(FMT_INCLUDE_DIR fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(fmt INTERFACE ${FMT_INCLUDE_DIR})
else()
  message(STATUS "Using third-party bundled fmt")
  include(ExternalProject)
  include(HPHPFunctions)

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    FMT_SOURCE_ARGS
    SOURCE_URL
    "https://github.com/fmtlib/fmt/releases/download/11.2.0/fmt-11.2.0.zip"
    SOURCE_HASH
    "SHA512=8b2a6793ab252bb31a63e54f808d6c48e2c6346ab5deaee7a1c4b5dc9ebc3faf493d31e5e1958e8fa0a6f9c6c0a7ec63ed778bd8d29f9e533bf8ffed281c1397"
  )

  set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/fmt-prefix/install")
  set(LIB_DIR "${INSTALL_DIR}/lib")
  if(MSVC)
    set(DEBUG_LIB_DIR "${INSTALL_DIR}/debug/lib")
  endif()

  ExternalProject_Add(
    bundled_fmt
    ${FMT_SOURCE_ARGS}
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_INSTALL_INCLUDEDIR=include
      -DCMAKE_INSTALL_LIBDIR=lib
      -DBUILD_SHARED_LIBS=OFF
      -DFMT_DOC=OFF
      -DFMT_TEST=OFF
      -DFMT_INSTALL=ON
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
  )
  add_dependencies(fmt bundled_fmt)

  target_include_directories(fmt INTERFACE "${INSTALL_DIR}/include")
  target_link_libraries(fmt INTERFACE "${LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}fmt${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()
