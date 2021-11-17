add_library(libzip INTERFACE)

# libzip 1.5.2 needed for https://github.com/nih-at/libzip/commit/c8951d8e8678c0d850664a497d2c2f94cc623240
find_package(LibZip)
unset(LIBZIP_FOUND)
if (LIBZIP_INCLUDE_DIR_ZIP AND LIBZIP_INCLUDE_DIR_ZIPCONF)
  if (LIBZIP_VERSION VERSION_LESS "1.5.2")
    message(STATUS "libzip is too old, found ${LIBZIP_VERSION} and we need 1.5.2+")
  else ()
    set(LIBZIP_FOUND TRUE)
    target_include_directories(libzip INTERFACE
      ${LIBZIP_INCLUDE_DIR_ZIP} ${LIBZIP_INCLUDE_DIR_ZIPCONF})
    target_link_libraries(libzip INTERFACE ${LIBZIP_LIBRARY})
    message(STATUS "Found libzip: ${LIBZIP_LIBRARY} ${LIBZIP_VERSION}")
    return()
  endif ()
endif ()

message(STATUS "Using third-party bundled libzip")

include(ExternalProject)
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  LIBZIP_DOWNLOAD_ARGS
  SOURCE_URL
  "https://libzip.org/download/libzip-1.6.1.tar.gz"
  SOURCE_HASH
  "SHA256=06eb8e9141fd19e2788cabaea9c9c2fd4d488d9e1484eb474bbfcac78e7b1d88"
)

ExternalProject_Add(
  bundled_libzip
  ${LIBZIP_DOWNLOAD_ARGS}
  EXCLUDE_FROM_ALL
  CMAKE_ARGS
  -DENABLE_COMMONCRYPTO=FALSE
  -DENABLE_GNUTLS=FALSE
  -DENABLE_MBEDTLS=FALSE
  -DENABLE_OPENSSL=FALSE
  -DENABLE_WINDOWS_CRYPTO=FALSE
  -DENABLE_BZIP2=FALSE
  -DENABLE_LZMA=FALSE
  -DBUILD_TOOLS=FALSE
  -DBUILD_REGRESS=FALSE
  -DBUILD_EXAMPLES=FALSE
  -DBUILD_DOC=FALSE
  -DBUILD_SHARED_LIBS=FALSE
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_INSTALL_INCLUDEDIR=include
  -DCMAKE_INSTALL_LIBDIR=lib
)

ExternalProject_Get_Property(bundled_libzip INSTALL_DIR)
add_dependencies(libzip bundled_libzip)
target_include_directories(libzip INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(
  libzip
  INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zip${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
