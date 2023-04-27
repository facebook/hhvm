# Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
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

# cmake -DWITH_ZSTD=system|bundled|path_to_zstd
# system is the default

SET(LIBZSTD_VERSION_REQUIRED "1.4.0")  # ZSTD_reset_session_only

MACRO (CHECK_ZSTD_VERSION)
  SET(PATH_TO_ZSTD_H "${ARGV0}/zstd.h")
  IF (NOT EXISTS ${PATH_TO_ZSTD_H})
    MESSAGE(FATAL_ERROR "File ${PATH_TO_ZSTD_H} not found")
  ENDIF()

  FILE(STRINGS "${PATH_TO_ZSTD_H}" LIBZSTD_HEADER_CONTENT REGEX "#define ZSTD_VERSION_[A-Z]+ +[0-9]+")
  STRING(REGEX REPLACE ".*#define ZSTD_VERSION_MAJOR +([0-9]+).*" "\\1" LIBZSTD_VERSION_MAJOR "${LIBZSTD_HEADER_CONTENT}")
  STRING(REGEX REPLACE ".*#define ZSTD_VERSION_MINOR +([0-9]+).*" "\\1" LIBZSTD_VERSION_MINOR "${LIBZSTD_HEADER_CONTENT}")
  STRING(REGEX REPLACE ".*#define ZSTD_VERSION_RELEASE +([0-9]+).*" "\\1" LIBZSTD_VERSION_RELEASE "${LIBZSTD_HEADER_CONTENT}")
  SET(LIBZSTD_VERSION_STRING "${LIBZSTD_VERSION_MAJOR}.${LIBZSTD_VERSION_MINOR}.${LIBZSTD_VERSION_RELEASE}")
  UNSET(LIBZSTD_HEADER_CONTENT)

  IF (${LIBZSTD_VERSION_STRING} VERSION_LESS ${LIBZSTD_VERSION_REQUIRED})
    MESSAGE(FATAL_ERROR "Required libzstd ${LIBZSTD_VERSION_REQUIRED} and installed version is ${LIBZSTD_VERSION_STRING}")
  ELSE()
    MESSAGE(STATUS "Found libzstd version ${LIBZSTD_VERSION_STRING}")
  ENDIF()
ENDMACRO()

MACRO (FIND_SYSTEM_ZSTD)
  FIND_PATH(PATH_TO_ZSTD
    NAMES zstd.h
    PATH_SUFFIXES include)
  FIND_LIBRARY(ZSTD_SYSTEM_LIBRARY
    NAMES zstd
    PATH_SUFFIXES lib)
  IF (PATH_TO_ZSTD AND ZSTD_SYSTEM_LIBRARY)
    SET(SYSTEM_ZSTD_FOUND 1)
    CHECK_ZSTD_VERSION(${PATH_TO_ZSTD})
    SET(ZSTD_INCLUDE_DIR ${PATH_TO_ZSTD})
    SET(ZSTD_LIBRARY ${ZSTD_SYSTEM_LIBRARY})
    MESSAGE(STATUS "ZSTD_INCLUDE_DIR ${ZSTD_INCLUDE_DIR}")
    MESSAGE(STATUS "ZSTD_LIBRARY(system): ${ZSTD_LIBRARY}")
  ENDIF()
ENDMACRO()

MACRO (MYSQL_USE_BUNDLED_ZSTD)
  SET(WITH_ZSTD "bundled" CACHE STRING "By default use bundled zstd library")
  SET(BUILD_BUNDLED_ZSTD 1)
  SET(ZSTD_LIBRARY zstd CACHE INTERNAL "Bundled zlib library")
  CHECK_ZSTD_VERSION(${CMAKE_SOURCE_DIR}/extra/zstd/lib)
  INCLUDE_DIRECTORIES(BEFORE SYSTEM ${CMAKE_SOURCE_DIR}/extra/zstd/lib
                                    ${CMAKE_SOURCE_DIR}/extra/zstd/lib/dictBuilder)
  ADD_SUBDIRECTORY(extra/zstd)
  MESSAGE(STATUS "ZSTD_LIBRARY(Bundled): " ${ZSTD_LIBRARY})
ENDMACRO()

IF (NOT WITH_ZSTD)
  SET(WITH_ZSTD "system" CACHE STRING "By default use system zstd library")
ENDIF()

MACRO (MYSQL_CHECK_ZSTD)
  # See if WITH_ZSTD is of the form </path/to/custom/installation>
  FILE(GLOB WITH_ZSTD_HEADER ${WITH_ZSTD}/include/zstd.h)
  IF (WITH_ZSTD_HEADER)
    SET(WITH_ZSTD_PATH ${WITH_ZSTD}
      CACHE PATH "Path to custom ZSTD installation")
  ENDIF()

  IF (WITH_ZSTD STREQUAL "bundled")
    MYSQL_USE_BUNDLED_ZSTD()
  ELSEIF(WITH_ZSTD STREQUAL "system")
    FIND_SYSTEM_ZSTD()
    IF (NOT SYSTEM_ZSTD_FOUND)
      MESSAGE(FATAL_ERROR "Cannot find system zstd libraries.")
    ENDIF()
  ELSEIF(WITH_ZSTD_PATH)
    # First search in WITH_ZSTD_PATH.
    FIND_PATH(ZSTD_ROOT_DIR
      NAMES include/zstd.h
      NO_CMAKE_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      HINTS ${WITH_ZSTD_PATH}
    )

    # Then search in standard places (if not found above).
    FIND_PATH(ZSTD_ROOT_DIR
      NAMES include/zstd.h
    )

    FIND_PATH(ZSTD_INCLUDE_DIR
      NAMES zstd.h
      HINTS ${ZSTD_ROOT_DIR}/include
    )

    FIND_LIBRARY(ZSTD_LIBRARY
      NAMES zstd_pic zstd
      HINTS ${ZSTD_ROOT_DIR}/lib)
    IF(ZSTD_INCLUDE_DIR AND ZSTD_LIBRARY)
      CHECK_ZSTD_VERSION(${ZSTD_INCLUDE_DIR})
      MESSAGE(STATUS "ZSTD_INCLUDE_DIR = ${ZSTD_INCLUDE_DIR}")
      MESSAGE(STATUS "ZSTD_LIBRARY = ${ZSTD_LIBRARY}")
    ELSE()
      MESSAGE(FATAL_ERROR "Cannot find appropriate libraries for zstd for WITH_ZSTD=${WITH_ZSTD}.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "Cannot find appropriate libraries for zstd.")
  ENDIF()
ENDMACRO()
