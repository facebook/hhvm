add_library(fbmysqlclient INTERFACE)

cmake_minimum_required(VERSION 2.8.0)
include(ExternalProject)

if (NOT TARGET lz4)
  message(FATAL_ERROR "third-party/lz4 must be processed before third-party/fb-mysql")
endif ()

# If we're building lz4, pass down its install path to fb-mysql. Otherwise
# (i.e., if we're using the system's lz4), no need to pass down anything,
# fb-mysql will also find and use the system's lz4. This assumes that HHVM's
# minimum lz4 version >= fb-mysql's minimum version.
set(EXTRA_ARGS "")
if (TARGET lz4Build)
  ExternalProject_Get_Property(lz4Build INSTALL_DIR)
  list(APPEND EXTRA_ARGS -DWITH_LZ4=3rdparty -DLZ4_PATH=${INSTALL_DIR})
endif ()

MYSQL_SOCKET_SEARCH()
if (NOT MYSQL_UNIX_SOCK_ADDR)
  message(FATAL_ERROR "Could not find MySQL socket path - if you install a MySQL server, this should be automatically detected. Alternatively, specify -DMYSQL_UNIX_SOCK_ADDR=/path/to/mysql.socket ; if you don't care about unix socket support for MySQL, specify -DMYSQL_UNIX_SOCK_ADDR=/dev/null")
endif ()

ExternalProject_Add(
  fbmysqlclientBuild
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/
  CMAKE_ARGS
  -DWITHOUT_SERVER=TRUE
  -DDISABLE_SHARED=TRUE
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
  -DOPENSSL_LIBRARY=${OPENSSL_SSL_LIBRARY}
  -DCRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
  -DMYSQL_UNIX_ADDR=${MYSQL_UNIX_SOCK_ADDR}
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_INSTALL_INCLUDEDIR=include
  -DCMAKE_INSTALL_LIBDIR=lib
  ${EXTRA_ARGS}
  BUILD_COMMAND $(MAKE) -C libmysql
)

# TODO: re2, zstd are also dependencies, but first they need to be split into
# re2 + re2Build, zstd + zstdBuild -- otherwise putting them here would force
# them to always be built, instead of using the system ones where available.
set(DEPS lz4)
add_dependencies(fbmysqlclientBuild ${DEPS})

# This extra indirection is needed because if the fbmysqlclient target links
# ${DEPS} directly, the correct linking order is not guaranteed.
add_library(fbmysqlclientDeps INTERFACE)
target_link_libraries(fbmysqlclientDeps INTERFACE ${DEPS})

ExternalProject_Get_Property(fbmysqlclientBuild INSTALL_DIR)

add_dependencies(fbmysqlclient fbmysqlclientBuild)
target_include_directories(fbmysqlclient INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(fbmysqlclient INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mysqlclient${CMAKE_STATIC_LIBRARY_SUFFIX}"
  fbmysqlclientDeps
)
