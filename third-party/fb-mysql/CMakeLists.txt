add_library(fbmysqlclient INTERFACE)

cmake_minimum_required(VERSION 2.8.0)
include(ExternalProject)

MYSQL_SOCKET_SEARCH()
if (NOT MYSQL_UNIX_SOCK_ADDR)
  message(FATAL_ERROR "Could not find MySQL socket path - if you install a MySQL server, this should be automatically detected. Alternatively, specify -DMYSQL_UNIX_SOCK_ADDR=/path/to/mysql.socket ; if you don't care about unix socket support for MySQL, specify -DMYSQL_UNIX_SOCK_ADDR=/dev/null")
endif ()

include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  FB_MYSQL_DOWNLOAD_ARGS
  SOURCE_URL
  "https://github.com/facebook/mysql-5.6/archive/refs/tags/fb-prod202104.tar.gz"
  SOURCE_HASH
  "SHA512=67cc99a8896719fc45493c53c92a1307cb4999aa725f4e9fb8c17f255353dab0fe2c681a045c52995ff3b56b4881a373838408491672a45cde053d3851db69d7"
)
ExternalProject_Add(
  bundled_fbmysqlclient
  ${FB_MYSQL_DOWNLOAD_ARGS}
  PATCH_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/../apply-quilt-patches.sh"
  "${CMAKE_CURRENT_SOURCE_DIR}/patches"
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
  -DWITH_LZ4=system
  "-DLZ4_SYSTEM_LIBRARY=$<TARGET_PROPERTY:lz4,INTERFACE_LINK_LIBRARIES>"
  "-DPATH_TO_LZ4=$<TARGET_PROPERTY:lz4,INTERFACE_INCLUDE_DIRECTORIES>"
  -DWITH_ZSTD=system
  "-DZSTD_SYSTEM_LIBRARY=$<TARGET_PROPERTY:zstd,INTERFACE_LINK_LIBRARIES>"
  "-DPATH_TO_ZSTD=$<TARGET_PROPERTY:zstd,INTERFACE_INCLUDE_DIRECTORIES>"
  ${EXTRA_ARGS}
)

# TODO: re2 is also a dependency, but first it needs to be split into
# re2 + re2Build -- otherwise putting them here would force
# them to always be built, instead of using the system ones where available.
set(DEPS lz4 zstd)
add_dependencies(bundled_fbmysqlclient ${DEPS})

# This extra indirection is needed because if the fbmysqlclient target links
# ${DEPS} directly, the correct linking order is not guaranteed.
add_library(fbmysqlclientDeps INTERFACE)
target_link_libraries(fbmysqlclientDeps INTERFACE ${DEPS})

ExternalProject_Get_Property(bundled_fbmysqlclient INSTALL_DIR)

add_dependencies(fbmysqlclient bundled_fbmysqlclient)
target_include_directories(fbmysqlclient INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(fbmysqlclient INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mysqlclient${CMAKE_STATIC_LIBRARY_SUFFIX}"
  fbmysqlclientDeps
)
