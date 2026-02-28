# Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
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

MACRO(GET_CURL_VERSION)
  FILE(STRINGS "${CURL_INCLUDE_DIR}/curl/curlver.h"
    CURL_VERSION_NUMBERS
    REGEX "^#[ ]*define[\t ]+LIBCURL_VERSION_[A-Z]+[\t ]+[0-9].*"
    )
  STRING(REGEX REPLACE
    "^.*LIBCURL_VERSION_MAJOR[\t ]+([0-9]+).*$" "\\1"
    CURL_VERSION_MAJOR "${CURL_VERSION_NUMBERS}"
    )
  STRING(REGEX REPLACE
    "^.*LIBCURL_VERSION_MINOR[\t ]+([0-9]+).*$" "\\1"
    CURL_VERSION_MINOR "${CURL_VERSION_NUMBERS}"
    )
  MESSAGE(STATUS "CURL version: ${CURL_VERSION_MAJOR}.${CURL_VERSION_MINOR}")
ENDMACRO()

MACRO(MYSQL_CHECK_CURL)
  IF(WITH_CURL STREQUAL "system")
    #  FindCURL.cmake will set
    #  CURL_INCLUDE_DIRS   - where to find curl/curl.h, etc.
    #  CURL_LIBRARIES      - List of libraries when using curl.
    #  CURL_FOUND          - True if curl found.
    #  CURL_VERSION_STRING - the version of curl found (since CMake 2.8.8)
    FIND_PACKAGE(CURL)
    IF(CURL_FOUND AND
        CURL_LIBRARIES AND
        NOT CURL_LIBRARIES MATCHES "CURL_LIBRARY-NOTFOUND" AND
        NOT CURL_INCLUDE_DIRS MATCHES "CURL_INCLUDE_DIR-NOTFOUND")
      SET(CURL_LIBRARY ${CURL_LIBRARIES} CACHE FILEPATH "Curl library")
      SET(CURL_INCLUDE_DIR ${CURL_INCLUDE_DIRS} CACHE PATH "Curl include")
      GET_CURL_VERSION()
    ELSE()
      SET(CURL_LIBRARY "")
      SET(CURL_INCLUDE_DIR "")
    ENDIF()
    MESSAGE(STATUS "CURL_LIBRARY = ${CURL_LIBRARY}")
    MESSAGE(STATUS "CURL_INCLUDE_DIR = ${CURL_INCLUDE_DIR}")

  ELSEIF(WITH_CURL STREQUAL "bundled")
    MESSAGE(FATAL_ERROR "There is no bundled CURL library.")

  ELSEIF(WITH_CURL)
    # Explicit path given. Normalize path for the following regex replace.
    FILE(TO_CMAKE_PATH "${WITH_CURL}" WITH_CURL)
    # Pushbuild adds /lib to the CURL path
    STRING(REGEX REPLACE "/lib$" "" WITH_CURL "${WITH_CURL}")
    LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
    FIND_LIBRARY(CURL_LIBRARY
      NAMES curl libcurl
      PATHS ${WITH_CURL} ${WITH_CURL}/lib
      NO_DEFAULT_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      )
    LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
    IF(CURL_LIBRARY MATCHES "CURL_LIBRARY-NOTFOUND")
      MESSAGE(FATAL_ERROR "CURL library not found under '${WITH_CURL}'")
    ENDIF()
    FIND_PATH(CURL_INCLUDE_DIR
      NAMES curl/curl.h
      PATHS ${WITH_CURL} ${WITH_CURL}/include
      NO_DEFAULT_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      )
    IF(CURL_INCLUDE_DIR MATCHES "CURL_INCLUDE_DIR-NOTFOUND")
      MESSAGE(FATAL_ERROR "CURL include files not found under '${WITH_CURL}'")
    ENDIF()
    SET(WITH_CURL_PATH ${WITH_CURL} CACHE PATH "path to CURL installation")
    SET(WITH_CURL_PATH ${WITH_CURL})
    GET_CURL_VERSION()
    MESSAGE(STATUS "CURL_LIBRARY = ${CURL_LIBRARY}")
    MESSAGE(STATUS "CURL_INCLUDE_DIR = ${CURL_INCLUDE_DIR}")

  ELSE()
    MESSAGE(STATUS "No WITH_CURL has been set. Not using any curl library.")
    SET(CURL_LIBRARY "")
    SET(CURL_INCLUDE_DIR "")
    MESSAGE(STATUS "CURL_LIBRARY = ${CURL_LIBRARY}")
    MESSAGE(STATUS "CURL_INCLUDE_DIR = ${CURL_INCLUDE_DIR}")
  ENDIF()
ENDMACRO()


MACRO(MYSQL_CHECK_CURL_DLLS)

  IF (WITH_CURL_PATH AND WIN32)

    MESSAGE(STATUS "WITH_CURL_PATH ${WITH_CURL_PATH}")
    GET_FILENAME_COMPONENT(CURL_NAME "${CURL_LIBRARY}" NAME_WE)
    FIND_FILE(HAVE_CURL_DLL
      NAMES "${CURL_NAME}.dll"
      PATHS "${WITH_CURL_PATH}/lib"
      NO_DEFAULT_PATH
      )
    MESSAGE(STATUS "HAVE_CURL_DLL ${HAVE_CURL_DLL}")
    IF(HAVE_CURL_DLL)
      SET(CURL_LIBRARY "CURL_LIBRARY-NOTFOUND")
      FIND_LIBRARY(CURL_LIBRARY
        NAMES libcurl_imp.lib
        PATHS ${WITH_CURL} ${WITH_CURL}/lib
        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        )
      IF(CURL_LIBRARY MATCHES "CURL_LIBRARY-NOTFOUND")
        MESSAGE(FATAL_ERROR
          "CURL dll import library not found under '${WITH_CURL}'")
      ENDIF()
      GET_FILENAME_COMPONENT(CURL_DLL_NAME "${HAVE_CURL_DLL}" NAME)
      ADD_CUSTOM_TARGET(copy_curl_dlls ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${HAVE_CURL_DLL}"
        "${CMAKE_BINARY_DIR}/runtime_output_directory/${CMAKE_CFG_INTDIR}/${CURL_DLL_NAME}"
        )

      MESSAGE(STATUS "INSTALL ${HAVE_CURL_DLL} to ${INSTALL_BINDIR}")
      INSTALL(FILES "${HAVE_CURL_DLL}"
        DESTINATION "${INSTALL_BINDIR}" COMPONENT SharedLibraries)

      SET(ZLIB_DLL_REQUIRED 1)
      FIND_OBJECT_DEPENDENCIES("${HAVE_CURL_DLL}" DEPENDENCY_LIST)
      LIST(FIND DEPENDENCY_LIST "zlib.dll" FOUNDIT1)
      LIST(FIND DEPENDENCY_LIST "zlib1.dll" FOUNDIT2)
      MESSAGE(STATUS "${CURL_DLL_NAME} DEPENDENCY_LIST ${DEPENDENCY_LIST}")
      IF(FOUNDIT1 LESS 0 AND FOUNDIT2 LESS 0)
        UNSET(ZLIB_DLL_REQUIRED)
      ENDIF()

      FIND_FILE(HAVE_ZLIB_DLL
        NAMES zlib.dll zlib1.dll
        PATHS "${WITH_CURL_PATH}/lib"
        NO_DEFAULT_PATH
        )
      MESSAGE(STATUS "HAVE_ZLIB_DLL ${HAVE_ZLIB_DLL}")

      IF(ZLIB_DLL_REQUIRED AND NOT HAVE_ZLIB_DLL)
        MESSAGE(FATAL_ERROR "libcurl.dll depends on zlib.dll or zlib1.dll")
      ENDIF()

      IF(ZLIB_DLL_REQUIRED AND HAVE_ZLIB_DLL)
        MESSAGE(STATUS "INSTALL ${HAVE_ZLIB_DLL} to ${INSTALL_BINDIR}")
        INSTALL(FILES "${HAVE_ZLIB_DLL}"
          DESTINATION "${INSTALL_BINDIR}" COMPONENT SharedLibraries)
        GET_FILENAME_COMPONENT(ZLIB_DLL_NAME "${HAVE_ZLIB_DLL}" NAME)
        ADD_CUSTOM_TARGET(copy_zlib_dlls ALL
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${HAVE_ZLIB_DLL}"
          "${CMAKE_BINARY_DIR}/runtime_output_directory/${CMAKE_CFG_INTDIR}/${ZLIB_DLL_NAME}"
          )
        ADD_DEPENDENCIES(copy_curl_dlls copy_zlib_dlls)
      ENDIF()
    ELSE()
      MESSAGE(STATUS "Cannot find CURL dynamic libraries")
    ENDIF()

  ENDIF()
ENDMACRO()
