# Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.
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
 
#
# Headers which need to be checked for abi/api compatibility are in
# API_PREPROCESSOR_HEADER. plugin.h is tested implicitly via
# plugin_audit.h and plugin_ftparser.h.
#
# We use gcc specific preprocessing command and sed/diff, so it will 
# only be run  on Unix and only if gcc is used. On some Unixes,
# (Solaris) sed or diff might act differently from GNU, so we run only 
# on systems we can trust.
IF(LINUX)
  SET(RUN_ABI_CHECK 1)
ELSE()
  SET(RUN_ABI_CHECK 0)
ENDIF()

IF(MY_COMPILER_IS_GNU AND RUN_ABI_CHECK)
  IF(CMAKE_C_COMPILER MATCHES "ccache$")
    SET(COMPILER ${CMAKE_C_COMPILER_ARG1})
    STRING(REGEX REPLACE "^ " "" COMPILER ${COMPILER})
  ELSE()
    SET(COMPILER ${CMAKE_C_COMPILER})
  ENDIF()
  SET(API_PREPROCESSOR_HEADER
    ${CMAKE_SOURCE_DIR}/include/mysql/plugin_audit.h
    ${CMAKE_SOURCE_DIR}/include/mysql/plugin_ftparser.h
    ${CMAKE_SOURCE_DIR}/include/mysql.h
    ${CMAKE_SOURCE_DIR}/include/mysql/client_plugin.h
    ${CMAKE_SOURCE_DIR}/include/mysql/plugin_auth.h
    ${CMAKE_SOURCE_DIR}/include/mysql/plugin_keyring.h
  )
  IF(NOT WITHOUT_SERVER)
    SET(API_PREPROCESSOR_HEADER
      ${API_PREPROCESSOR_HEADER}
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_thread_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_mutex_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_rwlock_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_cond_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_file_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_socket_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_table_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_mdl_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_idle_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_stage_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_statement_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_transaction_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_memory_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_error_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/psi/psi_abi_system_v1.h
      ${CMAKE_SOURCE_DIR}/include/mysql/services.h
    )
  ENDIF()

  ADD_CUSTOM_TARGET(abi_check ALL
  COMMAND ${CMAKE_COMMAND} 
    -DCOMPILER=${COMPILER}
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DBINARY_DIR=${CMAKE_BINARY_DIR}
    "-DABI_HEADERS=${API_PREPROCESSOR_HEADER}"
    -P ${CMAKE_SOURCE_DIR}/cmake/do_abi_check.cmake
    VERBATIM
  )

  ADD_CUSTOM_TARGET(abi_check_all
  COMMAND ${CMAKE_COMMAND} 
    -DCOMPILER=${COMPILER} 
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DBINARY_DIR=${CMAKE_BINARY_DIR}
    "-DABI_HEADERS=${API_PREPROCESSOR_HEADER}"
    -P ${CMAKE_SOURCE_DIR}/cmake/do_abi_check.cmake
    VERBATIM
  )
ENDIF()

