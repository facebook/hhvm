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

# Add executable plus some additional MySQL specific stuff
# Usage (same as for standard CMake's ADD_EXECUTABLE)
#
# MYSQL_ADD_EXECUTABLE(target source1...sourceN)
# MySQL specifics:
# - instruct CPack to install executable under
#   ${CMAKE_INSTALL_PREFIX}/bin directory
#
#   SKIP_INSTALL do not install it
#   ADD_TEST     add a unit test with given name (and add SKIP_INSTALL)
# On Windows :
# - add version resource
#
# All executables are built in ${CMAKE_BINARY_DIR}/runtime_output_directory
# (can be overridden by the RUNTIME_OUTPUT_DIRECTORY option).
# This is primarily done to simplify usage of dynamic libraries on Windows.
# It also simplifies test tools like mtr, which have to locate executables in
# order to run them during testing.

INCLUDE(cmake_parse_arguments)

FUNCTION (MYSQL_ADD_EXECUTABLE)
  # Pass-through arguments for ADD_EXECUTABLE
  MYSQL_PARSE_ARGUMENTS(ARG
   "DESTINATION;COMPONENT;ADD_TEST;DEPENDENCIES;LINK_LIBRARIES;RUNTIME_OUTPUT_DIRECTORY"
   "SKIP_INSTALL;ENABLE_EXPORTS;EXCLUDE_FROM_ALL;EXCLUDE_ON_SOLARIS"
   ${ARGN}
  )
  LIST(GET ARG_DEFAULT_ARGS 0 target)
  LIST(REMOVE_AT  ARG_DEFAULT_ARGS 0)

  # Collect all executables in the same directory
  IF(ARG_RUNTIME_OUTPUT_DIRECTORY)
    SET(TARGET_RUNTIME_OUTPUT_DIRECTORY ${ARG_RUNTIME_OUTPUT_DIRECTORY})
  ELSE()
    SET(TARGET_RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_BINARY_DIR}/runtime_output_directory)
  ENDIF()

  SET(sources ${ARG_DEFAULT_ARGS})
  ADD_VERSION_INFO(${target} EXECUTABLE sources)

  ADD_EXECUTABLE(${target} ${sources})

  SET_PATH_TO_SSL(${target} ${TARGET_RUNTIME_OUTPUT_DIRECTORY})

  IF(ARG_DEPENDENCIES)
    ADD_DEPENDENCIES(${target} ${ARG_DEPENDENCIES})
  ENDIF()
  IF(ARG_COMPONENT STREQUAL "Router")
    ADD_DEPENDENCIES(mysqlrouter_all ${target})
  ENDIF()

  IF(ARG_LINK_LIBRARIES)
    TARGET_LINK_LIBRARIES(${target} ${ARG_LINK_LIBRARIES})
  ENDIF()

  IF(SOLARIS AND ARG_EXCLUDE_ON_SOLARIS)
    MESSAGE(WARNING
      "Likely link failure for this compiler, skipping target ${target}")
    SET(ARG_EXCLUDE_FROM_ALL TRUE)
    SET(ARG_SKIP_INSTALL TRUE)
    UNSET(ARG_ADD_TEST)
  ENDIF()

  IF(ARG_ENABLE_EXPORTS)
    SET_TARGET_PROPERTIES(${target} PROPERTIES ENABLE_EXPORTS TRUE)
  ENDIF()

  IF(ARG_EXCLUDE_FROM_ALL)
#   MESSAGE(STATUS "EXCLUDE_FROM_ALL ${target}")
    SET_PROPERTY(TARGET ${target} PROPERTY EXCLUDE_FROM_ALL TRUE)
    IF(WIN32)
      SET_PROPERTY(TARGET ${target} PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    ENDIF()
  ENDIF()

  SET_TARGET_PROPERTIES(${target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TARGET_RUNTIME_OUTPUT_DIRECTORY})

  IF(WIN32_CLANG AND WITH_ASAN)
    TARGET_LINK_LIBRARIES(${target} "${ASAN_LIB_DIR}/clang_rt.asan-x86_64.lib")
    TARGET_LINK_LIBRARIES(${target} "${ASAN_LIB_DIR}/clang_rt.asan_cxx-x86_64.lib")
    SET_TARGET_PROPERTIES(${target} PROPERTIES LINK_FLAGS
      "/wholearchive:\"${ASAN_LIB_DIR}/clang_rt.asan-x86_64.lib\" /wholearchive:\"${ASAN_LIB_DIR}/clang_rt.asan_cxx-x86_64.lib\"")
  ENDIF()

  # Add unit test, do not install it.
  IF (ARG_ADD_TEST)
    ADD_TEST(${ARG_ADD_TEST}
      ${TARGET_RUNTIME_OUTPUT_DIRECTORY}/${target})
    SET(ARG_SKIP_INSTALL TRUE)
  ENDIF()

  # tell CPack where to install
  IF(NOT ARG_SKIP_INSTALL)
    IF(NOT ARG_DESTINATION)
      SET(ARG_DESTINATION ${INSTALL_BINDIR})
    ENDIF()
    IF(ARG_COMPONENT)
      SET(COMP COMPONENT ${ARG_COMPONENT})
    ELSE()
      SET(COMP COMPONENT Client)
    ENDIF()
    ADD_INSTALL_RPATH_FOR_OPENSSL(${target})
    MYSQL_INSTALL_TARGETS(${target} DESTINATION ${ARG_DESTINATION} ${COMP})
#   MESSAGE(STATUS "INSTALL ${target} ${ARG_DESTINATION}")
  ENDIF()
ENDFUNCTION()
