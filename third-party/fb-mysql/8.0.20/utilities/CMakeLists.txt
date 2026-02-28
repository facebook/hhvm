# Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

ADD_WSHADOW_WARNING()
DISABLE_MISSING_PROFILE_WARNING()

IF(NOT CMAKE_CROSSCOMPILING)
  MYSQL_ADD_EXECUTABLE(comp_err
    comp_err.cc
    COMPONENT Test
    LINK_LIBRARIES mysys
    )
ENDIF()

MYSQL_ADD_EXECUTABLE(comp_client_err
  comp_client_err.cc
  ${CMAKE_SOURCE_DIR}/libmysql/errmsg.cc
  LINK_LIBRARIES mysys
  SKIP_INSTALL
  )

ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/include/mysqlclient_ername.h
  COMMAND comp_client_err
  --in_file=${PROJECT_SOURCE_DIR}/include/errmsg.h
  --out_file=${PROJECT_BINARY_DIR}/include/mysqlclient_ername.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/comp_client_err.cc)

ADD_CUSTOM_TARGET(GenClientError ALL
  DEPENDS ${PROJECT_BINARY_DIR}/include/mysqlclient_ername.h)


IF(BUILD_BUNDLED_LZ4)
  ADD_LIBRARY(lz4_lib STATIC
    ../extra/lz4/lz4.c
    ../extra/lz4/lz4frame.c
    ../extra/lz4/lz4hc.c
    ../extra/lz4/xxhash.c
  )
ENDIF()

FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${INSTALL_MYSQLSHAREDIR})

# Create localized error message files, <language>/errmsg.sys
#
# errmsg-utf8.txt was split into messages_to_clients.txt and
# messages_to_error_log.txt, respectively.
#
# As the names suggest, this covers messages sent by the server to
# the error-log, and those sent by the server to clients.
# For messages built into the clients (or rather, libmysql / C-API)
# itself, see the MYSQL_ADD_EXECUTABLE(comp_client_err ...) section.
#
# Please see errmsg_readme.txt in the same directory for more information.
#
ADD_CUSTOM_COMMAND(OUTPUT
  ${PROJECT_BINARY_DIR}/include/mysqld_error.h 
  ${PROJECT_BINARY_DIR}/include/mysqld_ername.h
  ${PROJECT_BINARY_DIR}/include/mysqld_errmsg.h
  ${PROJECT_BINARY_DIR}/${INSTALL_MYSQLSHAREDIR}/english/errmsg.sys
  COMMAND comp_err
  --charset=${PROJECT_SOURCE_DIR}/share/charsets
  --out-dir=${PROJECT_BINARY_DIR}/${INSTALL_MYSQLSHAREDIR}/
  --header_file=${PROJECT_BINARY_DIR}/include/mysqld_error.h 
  --name_file=${PROJECT_BINARY_DIR}/include/mysqld_ername.h
  --errmsg_file=${PROJECT_BINARY_DIR}/include/mysqld_errmsg.h
  --in_file_errlog=${PROJECT_SOURCE_DIR}/share/messages_to_error_log.txt
  --in_file_toclient=${PROJECT_SOURCE_DIR}/share/messages_to_clients.txt
  DEPENDS
  ${PROJECT_SOURCE_DIR}/share/messages_to_error_log.txt
  ${PROJECT_SOURCE_DIR}/share/messages_to_clients.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/comp_err.cc)


ADD_CUSTOM_TARGET(GenError ALL
  DEPENDS 
  ${PROJECT_BINARY_DIR}/include/mysqld_error.h
  ${PROJECT_BINARY_DIR}/include/mysqld_ername.h
  ${PROJECT_BINARY_DIR}/include/mysqld_errmsg.h
  ${PROJECT_BINARY_DIR}/${INSTALL_MYSQLSHAREDIR}/english/errmsg.sys
  ${PROJECT_SOURCE_DIR}/share/messages_to_error_log.txt
  ${PROJECT_SOURCE_DIR}/share/messages_to_clients.txt)

MYSQL_ADD_EXECUTABLE(range_check_err
  range_check_err.cc
  DEPENDENCIES GenError
  LINK_LIBRARIES mysys
  SKIP_INSTALL
  )

# Set InnoDB mutex type
ADD_DEFINITIONS(-DMUTEX_EVENT)

# The copied shared libraries must be available at build time.
IF(TARGET copy_openssl_dlls)
  ADD_DEPENDENCIES(GenError copy_openssl_dlls)
  ADD_DEPENDENCIES(comp_client_err copy_openssl_dlls)
  ADD_DEPENDENCIES(comp_err copy_openssl_dlls)
ENDIF()

MYSQL_ADD_EXECUTABLE(static_thread_local_test
  static_thread_local_test.cc
  SKIP_INSTALL
  EXCLUDE_FROM_ALL
  )

MYSQL_ADD_EXECUTABLE(my_print_defaults
  my_print_defaults.cc
  COMPONENT Server
  LINK_LIBRARIES mysys
  )
MYSQL_ADD_EXECUTABLE(perror
  perror.cc
  COMPONENT Server
  DEPENDENCIES GenError
  LINK_LIBRARIES mysys
  )
IF(BUILD_BUNDLED_LZ4)
  MYSQL_ADD_EXECUTABLE(lz4_decompress
    lz4_decompress.cc
    COMPONENT Server
    LINK_LIBRARIES ${LZ4_LIBRARY} mysys
    )
ENDIF()

IF(BUILD_BUNDLED_ZLIB OR NOT OPENSSL_EXECUTABLE_HAS_ZLIB)
  MYSQL_ADD_EXECUTABLE(zlib_decompress
    zlib_decompress.cc
    COMPONENT Server
    LINK_LIBRARIES ${ZLIB_LIBRARY} mysys
    )
ENDIF()

# All targets below belong to COMPONENT Server and depend on InnoDB.
IF(WITHOUT_SERVER)
  RETURN()
ENDIF()

ADD_DEFINITIONS(-DHAVE_LZ4=1)

IF(NOT MSVC)
  IF(NOT CMAKE_CROSSCOMPILING)
    CHECK_C_SOURCE_RUNS(
      "#include<stdint.h>
      int main()
      {
        unsigned char       a = 0;
        unsigned char       b = 0;
        unsigned char       c = 1;

        __atomic_exchange(&a, &b,  &c, __ATOMIC_RELEASE);
        __atomic_compare_exchange(&a, &b, &c, 0,
                                __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
        return(0);
      }"
      HAVE_IB_GCC_ATOMIC_COMPARE_EXCHANGE
      )
  ENDIF()

  IF(HAVE_IB_GCC_ATOMIC_COMPARE_EXCHANGE)
    ADD_DEFINITIONS(-DHAVE_IB_GCC_ATOMIC_COMPARE_EXCHANGE=1)
  ENDIF()
ENDIF(NOT MSVC)

# Add path to the InnoDB headers and DD
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/storage/innobase
  ${CMAKE_SOURCE_DIR}/storage/innobase/include
  ${CMAKE_SOURCE_DIR}/sql
  )

# We use the InnoDB code directly in case the code changes.
MYSQL_ADD_EXECUTABLE(innochecksum
  innochecksum.cc
  COMPONENT Server
  DEPENDENCIES GenError
  LINK_LIBRARIES mysys innodb_zipdecompress ${LZ4_LIBRARY}
  )
TARGET_COMPILE_DEFINITIONS(innochecksum PRIVATE UNIV_NO_ERR_MSGS UNIV_LIBRARY)
IF(MY_COMPILER_IS_GNU_OR_CLANG)
  ADD_COMPILE_FLAGS(innochecksum.cc
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-cast-qual")
ENDIF()

MYSQL_ADD_EXECUTABLE(ibd2sdi
  ibd2sdi.cc
  COMPONENT Server
  DEPENDENCIES GenError
  LINK_LIBRARIES mysys innodb_zipdecompress ${LZ4_LIBRARY}
  )
TARGET_COMPILE_DEFINITIONS(ibd2sdi PRIVATE UNIV_NO_ERR_MSGS UNIV_LIBRARY)
IF(MY_COMPILER_IS_GNU_OR_CLANG)
  ADD_COMPILE_FLAGS(ibd2sdi.cc
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-cast-qual")
ENDIF()
