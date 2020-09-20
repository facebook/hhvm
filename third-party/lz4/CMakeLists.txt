add_library(lz4 INTERFACE)

find_package(LZ4)  # also checks minimum required version

if (LZ4_FOUND)
  target_include_directories(lz4 INTERFACE ${LZ4_INCLUDE_DIR})
  target_link_libraries(lz4 INTERFACE ${LZ4_LIBRARY})

else (LZ4_FOUND)
  cmake_minimum_required(VERSION 2.8.0)
  include(ExternalProject)

  ExternalProject_Add(
    lz4Build
    URL "https://github.com/lz4/lz4/archive/v1.9.2.tar.gz"
    URL_HASH SHA256=658ba6191fa44c92280d4aa2c271b0f4fbc0e34d249578dd05e50e76d0e5efcc
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND make MOREFLAGS=-fPIC PREFIX=<INSTALL_DIR> install
  )

  ExternalProject_Get_Property(lz4Build INSTALL_DIR)

  add_dependencies(lz4 lz4Build)
  target_include_directories(lz4 INTERFACE "${INSTALL_DIR}/include")
  target_link_libraries(lz4 INTERFACE "${INSTALL_DIR}/lib/liblz4.a")

endif (LZ4_FOUND)
