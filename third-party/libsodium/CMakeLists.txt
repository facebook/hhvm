add_library(libsodium INTERFACE)
target_compile_definitions(libsodium INTERFACE -DHAVE_LIBSODIUM=1)
option(FORCE_BUNDLED_LIBSODIUM "Always build libsodium, instead of using the system version" OFF)

if (NOT FORCE_BUNDLED_LIBSODIUM)
  find_package(LibSodium 1.0.9)
endif ()

# Explicitly check FORCE_BUNDLED_LIBSODIUM again in case the other vars are cached
# from a previous run
if (LIBSODIUM_INCLUDE_DIRS AND LIBSODIUM_LIBRARIES AND NOT FORCE_BUNDLED_LIBSODIUM)
  message(STATUS "Using system libsodium ${LIBSODIUM_VERSION}")
  target_include_directories(libsodium INTERFACE ${LIBSODIUM_INCLUDE_DIRS})
  target_link_libraries(libsodium INTERFACE ${LIBSODIUM_LIBRARIES})
else ()
  message(STATUS "Using third-party bundled libsodium")

  include(ExternalProject)
  include(HPHPFunctions)

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    LIBSODIUM_DOWNLOAD_ARGS
    SOURCE_URL
    "https://github.com/jedisct1/libsodium/releases/download/1.0.18-RELEASE/libsodium-1.0.18.tar.gz"
    SOURCE_HASH
    "SHA512=17e8638e46d8f6f7d024fe5559eccf2b8baf23e143fadd472a7d29d228b186d86686a5e6920385fe2020729119a5f12f989c3a782afbd05a8db4819bb18666ef"
  )

  ExternalProject_Add(
    bundled_libsodium
    ${LIBSODIUM_DOWNLOAD_ARGS}
    CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      "--prefix=<INSTALL_DIR>"
      "--libdir=<INSTALL_DIR>/lib"
      "--includedir=<INSTALL_DIR>/include"
      --disable-debug
      --disable-dependency-tracking
      --disable-shared
      --enable-static
  )
  ExternalProject_Get_Property(bundled_libsodium INSTALL_DIR)
  target_include_directories(libsodium INTERFACE "${INSTALL_DIR}/include")
  add_dependencies(libsodium bundled_libsodium)
  target_link_libraries(
    libsodium
    INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sodium${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
endif ()
