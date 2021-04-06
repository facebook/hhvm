add_library(lz4 INTERFACE)

option(FORCE_BUNDLED_LZ4 "Always build lz4, instead of using the system version" ON)

if (NOT FORCE_BUNDLED_LZ4)
  find_package(LZ4)  # also checks minimum required version
endif ()

if (LZ4_FOUND AND NOT FORCE_BUNDLED_LZ4)
  target_include_directories(lz4 INTERFACE ${LZ4_INCLUDE_DIR})
  target_link_libraries(lz4 INTERFACE ${LZ4_LIBRARY})
else ()
  cmake_minimum_required(VERSION 2.8.0)
  include(ExternalProject)
  include(HPHPFunctions)

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    LZ4_DOWNLOAD_ARGS
    SOURCE_URL "https://github.com/lz4/lz4/archive/v1.9.2.tar.gz"
    SOURCE_HASH "SHA256=658ba6191fa44c92280d4aa2c271b0f4fbc0e34d249578dd05e50e76d0e5efcc"
    # By default, we'd look for v1.9.2.tar.gz in the cache - let's look for
    # lz4-v1.9.2 instead
    FILENAME_PREFIX "lz4-"
  )
  set(MAKE_ARGS MOREFLAGS=-fPIC)
  ExternalProject_Add(
    bundled_lz4
    ${LZ4_DOWNLOAD_ARGS}
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND \$\(MAKE\) ${MAKE_ARGS}
    INSTALL_COMMAND \$\(MAKE\) ${MAKE_ARGS} PREFIX=<INSTALL_DIR> install
  )

  ExternalProject_Get_Property(bundled_lz4 INSTALL_DIR)

  add_dependencies(lz4 bundled_lz4)
  target_include_directories(lz4 INTERFACE "${INSTALL_DIR}/include")
  target_link_libraries(
    lz4
    INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
endif ()
