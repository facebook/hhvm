add_library(brotli INTERFACE)
include(ExternalProject)

# Latest release is 1.0.7, and we need somthing newer (2.x? 1.1.x?), so use
# a specific git hash. This also means that we *always* need to use the bundled
# version.
ExternalProject_Add(
  bundled_brotli
  GIT_REPOSITORY https://github.com/google/brotli
  GIT_TAG 4b5771bee7995d3d606221caa455c044d80434df
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
)
add_dependencies(brotli bundled_brotli)

ExternalProject_Get_Property(bundled_brotli INSTALL_DIR)
target_include_directories(brotli INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(
  brotli
  INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlidec-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlienc-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlicommon-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
