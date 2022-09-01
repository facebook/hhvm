add_library(fbmysqlclient INTERFACE)

cmake_minimum_required(VERSION 2.8.0)
include(ExternalProject)

MYSQL_SOCKET_SEARCH()
if (NOT MYSQL_UNIX_SOCK_ADDR)
  message(FATAL_ERROR "Could not find MySQL socket path - if you install a MySQL server, this should be automatically detected. Alternatively, specify -DMYSQL_UNIX_SOCK_ADDR=/path/to/mysql.socket ; if you don't care about unix socket support for MySQL, specify -DMYSQL_UNIX_SOCK_ADDR=/dev/null")
endif ()

include(HPHPFunctions)

ExternalProject_Add(
  bundled_fbmysqlclient
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/
  PATCH_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/../apply-quilt-patches.sh"
  "${CMAKE_CURRENT_SOURCE_DIR}/patches"
  CMAKE_ARGS
  -DWITHOUT_SERVER=TRUE
  -DDISABLE_SHARED=TRUE
  -DWITH_PERFSCHEMA_STORAGE_ENGINE=ON
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
  -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
  -DOPENSSL_LIBRARY=${OPENSSL_SSL_LIBRARY}
  -DCRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
  -DMYSQL_UNIX_ADDR=${MYSQL_UNIX_SOCK_ADDR}
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_INSTALL_INCLUDEDIR=include
  -DCMAKE_INSTALL_LIBDIR=lib
  -DWITH_LZ4=system
  "-DLZ4_SYSTEM_LIBRARY=$<TARGET_PROPERTY:lz4,INTERFACE_LINK_LIBRARIES>"
  "-DPATH_TO_LZ4=$<TARGET_PROPERTY:lz4,INTERFACE_INCLUDE_DIRECTORIES>"
  -DWITH_RE2=system
  "-DRE2_SYSTEM_LIBRARY=$<TARGET_PROPERTY:re2,INTERFACE_LINK_LIBRARIES>"
  "-DPATH_TO_RE2=$<TARGET_PROPERTY:re2,INTERFACE_INCLUDE_DIRECTORIES>"
  -DWITH_ZSTD=system
  "-DZSTD_SYSTEM_LIBRARY=$<TARGET_PROPERTY:zstd,INTERFACE_LINK_LIBRARIES>"
  "-DPATH_TO_ZSTD=$<TARGET_PROPERTY:zstd,INTERFACE_INCLUDE_DIRECTORIES>"
  ${EXTRA_ARGS}
)

set(DEPS lz4 re2 zstd)
add_dependencies(bundled_fbmysqlclient ${DEPS})

# This extra indirection is needed because if the fbmysqlclient target links
# ${DEPS} directly, the correct linking order is not guaranteed.
add_library(fbmysqlclient_deps INTERFACE)
target_link_libraries(fbmysqlclient_deps INTERFACE ${DEPS})

ExternalProject_Get_Property(bundled_fbmysqlclient INSTALL_DIR)

add_dependencies(fbmysqlclient bundled_fbmysqlclient)
target_include_directories(fbmysqlclient INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(fbmysqlclient INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mysqlclient${CMAKE_STATIC_LIBRARY_SUFFIX}"
  fbmysqlclient_deps
)
