add_library(fbmysqlclient INTERFACE)

include(ExternalProject)

MYSQL_SOCKET_SEARCH()
if (NOT MYSQL_UNIX_SOCK_ADDR)
  message(FATAL_ERROR "Could not find MySQL socket path - if you install a MySQL server, this should be automatically detected. Alternatively, specify -DMYSQL_UNIX_SOCK_ADDR=/path/to/mysql.socket ; if you don't care about unix socket support for MySQL, specify -DMYSQL_UNIX_SOCK_ADDR=/dev/null")
endif ()

include(HPHPFunctions)

set(FBMYSQL_CMAKE_ARGS
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
  -DWITH_RE2=system
  -DWITH_ZSTD=system
  -DWITH_LIBEVENT=system
)

string(REPLACE ";" "|" FBMYSQL_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

ExternalProject_Add(
  bundled_fbmysqlclient
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/
  BUILD_ALWAYS ON
  LIST_SEPARATOR |
  CMAKE_GENERATOR "Unix Makefiles"
  CMAKE_ARGS
    ${FBMYSQL_CMAKE_ARGS}
    -DCMAKE_PREFIX_PATH=${FBMYSQL_PREFIX_PATH}
  BUILD_BYPRODUCTS "<INSTALL_DIR>/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mysqlclient${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

set(DEPS lz4 re2::re2 zstd)
add_dependencies(bundled_fbmysqlclient ${DEPS})

# This extra indirection is needed because if the fbmysqlclient target links
# ${DEPS} directly, the correct linking order is not guaranteed.
add_library(fbmysqlclient_deps INTERFACE)
target_link_libraries(fbmysqlclient_deps INTERFACE ${DEPS})

ExternalProject_Get_Property(bundled_fbmysqlclient INSTALL_DIR)

set(FBMYSQL_SOURCE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
set(FBMYSQL_COMPAT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/mysql-compat")
set(FBMYSQL_COMPAT_INCLUDE_DIR "${FBMYSQL_COMPAT_ROOT}/mysql/server/include")
file(MAKE_DIRECTORY "${FBMYSQL_COMPAT_ROOT}/mysql/server")
if (EXISTS "${FBMYSQL_COMPAT_INCLUDE_DIR}" OR IS_SYMLINK "${FBMYSQL_COMPAT_INCLUDE_DIR}")
  file(REMOVE_RECURSE "${FBMYSQL_COMPAT_INCLUDE_DIR}")
endif ()
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E create_symlink
    "${FBMYSQL_SOURCE_INCLUDE_DIR}"
    "${FBMYSQL_COMPAT_INCLUDE_DIR}"
  RESULT_VARIABLE FBMYSQL_SYMLINK_RESULT
)
if (NOT FBMYSQL_SYMLINK_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to create fb-mysql compatibility include symlink")
endif ()

add_dependencies(fbmysqlclient bundled_fbmysqlclient)
target_include_directories(
  fbmysqlclient
  INTERFACE
  "${INSTALL_DIR}/include"
  "${FBMYSQL_SOURCE_INCLUDE_DIR}"
  "${FBMYSQL_COMPAT_ROOT}"
)
target_link_libraries(fbmysqlclient INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mysqlclient${CMAKE_STATIC_LIBRARY_SUFFIX}"
  fbmysqlclient_deps
)
