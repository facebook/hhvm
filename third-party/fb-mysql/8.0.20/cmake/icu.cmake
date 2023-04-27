# Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
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

# We support different versions of ICU:
# - "bundled" uses source code in <source dir>/extra/icu
# - "system"  (typically) uses headers/libraries in /usr/lib and /usr/lib64
# - a custom installation of ICU can be used like this
#     - cmake -DCMAKE_PREFIX_PATH=</path/to/custom/icu> -DWITH_ICU="system"
#   or
#     - cmake -DWITH_ICU=</path/to/custom/icu>
#
# The default value for WITH_ICU is "bundled"
# set in cmake/build_configurations/feature_set.cmake

# To do: The default should probably be different depending on platform. On
# Windows, it should likely be wherever NuGet puts the libraries.

#
# install_root is either 'system' or is assumed to be a path.
#
MACRO (FIND_ICU install_root)
  IF("${install_root}" STREQUAL "system")
    SET(EXTRA_FIND_LIB_ARGS)
    SET(EXTRA_FIND_INC_ARGS)
  ELSE()
    SET(EXTRA_FIND_LIB_ARGS HINTS "${install_root}"
      PATH_SUFFIXES "lib" "lib64" NO_DEFAULT_PATH)
    SET(EXTRA_FIND_INC_ARGS HINTS "${install_root}"
      PATH_SUFFIXES "include"     NO_DEFAULT_PATH)
  ENDIF()

  FIND_PATH(ICU_INCLUDE_DIR NAMES unicode/regex.h ${EXTRA_FIND_INC_ARGS})
  IF (NOT ICU_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "Cannot find ICU regular expression headers")
  ENDIF()

  IF(WIN32)
    SET(ICU_LIBS icuuc icuio icudt icuin)
  ELSE()
    SET(ICU_LIBS icuuc icuio icudata icui18n)
  ENDIF()

  SET(ICU_SYSTEM_LIBRARIES)
  FOREACH(ICU_LIB ${ICU_LIBS})
    UNSET(ICU_LIB_PATH CACHE)
    FIND_LIBRARY(ICU_LIB_PATH NAMES ${ICU_LIB} ${EXTRA_FIND_LIB_ARGS})
    IF(NOT ICU_LIB_PATH)
      MESSAGE(FATAL_ERROR "Cannot find the ICU library ${ICU_LIB}")
    ENDIF()
    LIST(APPEND ICU_SYSTEM_LIBRARIES ${ICU_LIB_PATH})
  ENDFOREACH()

  # To do: If we include the path in ICU_INCLUDE_DIR, it leads to GUnit
  # picking up the wrong regex.h header. And it looks like we don't need it;
  # at least on Linux, the header gets installed in an OS path anyway.
  IF(NOT "${install_root}" STREQUAL "system")
    SET(ICU_INCLUDE_DIRS ${ICU_INCLUDE_DIR})
  ENDIF()

  SET(ICU_LIBRARIES ${ICU_SYSTEM_LIBRARIES})

  # Needed for version information.
  SET(ICU_COMMON_DIR ${ICU_INCLUDE_DIR})

ENDMACRO()

MACRO (MYSQL_USE_BUNDLED_ICU)
  SET(WITH_ICU "bundled" CACHE STRING "Use bundled icu library")
  SET(BUILD_BUNDLED_ICU 1)
  # To do: remove
  SET(UDATA_DEBUG 1)
  SET(ICU_DIR ${CMAKE_SOURCE_DIR}/extra/icu)
  SET(ICU_SOURCE_DIR ${ICU_DIR}/source)
  SET(ICU_COMMON_DIR ${ICU_SOURCE_DIR}/common)

  SET(ICU_INCLUDE_DIRS
    ${ICU_COMMON_DIR}
    ${ICU_SOURCE_DIR}/stubdata
    ${ICU_SOURCE_DIR}/i18n
  )
  # We do not want to set both DIR and DIRS, see MY_INCLUDE_SYSTEM_DIRECTORIES
  UNSET(ICU_INCLUDE_DIR)
  UNSET(ICU_INCLUDE_DIR CACHE)
  UNSET(ICU_LIB_PATH)
  UNSET(ICU_LIB_PATH CACHE)

  ADD_SUBDIRECTORY(${ICU_DIR})

  SET(ICU_LIBRARIES icui18n icuuc icustubdata)

ENDMACRO()

IF(NOT WITH_ICU)
  SET(WITH_ICU bundled CACHE STRING
    "By default use bundled icu library")
ENDIF()

MACRO (MYSQL_CHECK_ICU)
  ADD_DEFINITIONS(
    -DUNISTR_FROM_STRING_EXPLICIT=explicit
    -DUNISTR_FROM_CHAR_EXPLICIT=explicit

    # This will not work with a 'system' build. The shared libraries are
    # compiled with renaming turned on.
    #
    #    -DU_DISABLE_RENAMING
  )

  FILE(TO_CMAKE_PATH "${WITH_ICU}" WITH_ICU)

  IF (WITH_ICU STREQUAL "bundled")
    MYSQL_USE_BUNDLED_ICU()
  ELSEIF(WITH_ICU STREQUAL "system")
    FIND_ICU("system")
  ELSEIF(EXISTS "${WITH_ICU}")
    FIND_ICU("${WITH_ICU}")
  ELSE()
    MESSAGE(FATAL_ERROR "WITH_ICU must be 'bundled', 'system' or a path")
  ENDIF()

  # Extract the version number. Major version information looks like:
  #   #define U_ICU_VERSION_MAJOR_NUM nn
  FILE(STRINGS "${ICU_COMMON_DIR}/unicode/uvernum.h"
    ICU_MAJOR_VERSION_INFO
    REGEX "^#[ ]*define[\t ]+U_ICU_VERSION_MAJOR_NUM[\t ]+[0-9]+$"
  )
  STRING(REGEX REPLACE
    "^.*U_ICU_VERSION_MAJOR_NUM[\t ]+([0-9]+)$" "\\1"
    ICU_MAJOR_VERSION ${ICU_MAJOR_VERSION_INFO}
  )

  MESSAGE(STATUS "ICU_MAJOR_VERSION = ${ICU_MAJOR_VERSION}")
  MESSAGE(STATUS "ICU_INCLUDE_DIRS ${ICU_INCLUDE_DIRS}")
  MESSAGE(STATUS "ICU_LIBRARIES ${ICU_LIBRARIES}")

ENDMACRO()
