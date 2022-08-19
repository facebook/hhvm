# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
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

# cmake -DWITH_KERBEROS=system|<path/to/custom/installation>|none
# system is the default
# none will diable the kerberos build
# Custom path is only supported for LINUX_STANDALONE.

INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFiles)

SET(WITH_KERBEROS_DOC "\nsystem (use the OS sasl library)")
STRING_APPEND(WITH_KERBEROS_DOC ", \n</path/to/custom/installation>")
STRING_APPEND(WITH_KERBEROS_DOC ", \nnone (skip kerberos)>")

STRING(REPLACE "\n" "| " WITH_KERBEROS_DOC_STRING "${WITH_KERBEROS_DOC}")

MACRO(RESET_KERBEROS_VARIABLES)
  UNSET(KERBEROS_LIB_CONFIGURED CACHE)
  UNSET(KERBEROS_LIB_CONFIGURED)
  UNSET(KERBEROS_INCLUDE_DIR)
  UNSET(KERBEROS_INCLUDE_DIR CACHE)
  UNSET(KERBEROS_LIBRARIES)
  UNSET(KERBEROS_LIBRARIES CACHE)
ENDMACRO()

FUNCTION(WARN_MISSING_SYSTEM_KERBEROS OUTPUT_WARNING)
  IF(NOT KERBEROS_FOUND AND WITH_KERBEROS STREQUAL "system")
    MESSAGE(WARNING "Cannot find KERBEROS development libraries. "
      "You need to install the required packages:\n"
      "  Debian/Ubuntu:              apt install libkrb5-dev\n"
      "  RedHat/Fedora/Oracle Linux: yum install krb5-devel\n"
      "  SuSE:                       zypper install krb5-devel\n"
      )
    SET(${OUTPUT_WARNING} 1 PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()

MACRO(FIND_SYSTEM_KERBEROS)
  # Typical result of running pkg-config is:
  #   SYSTEM_KRB5_LDFLAGS      -lkrb5;-lk5crypto;-lcom_err
  #   SYSTEM_KRB5_INCLUDE_DIRS ""
  #   SYSTEM_KRB5_CFLAGS       ""
  # DEBIAN/UBUNTU:
  #   -L/usr/lib/x86_64-linux-gnu/mit-krb5;-lkrb5;-lk5crypto;-lcom_err
  #   ""
  #   -isystem;/usr/include/mit-krb5
  # FREEBSD :
  #   -L/usr/local/lib;-lkrb5;-lk5crypto;-lcom_err
  #   /usr/local/include
  #   -I/usr/local/include
  IF(LINUX OR FREEBSD)
    MYSQL_CHECK_PKGCONFIG()
    PKG_CHECK_MODULES(SYSTEM_KRB5 krb5)

    MESSAGE(STATUS "SYSTEM_KRB5_FOUND  ${SYSTEM_KRB5_FOUND}")
    MESSAGE(STATUS "SYSTEM_KRB5_LIBRARIES  ${SYSTEM_KRB5_LIBRARIES}")
    MESSAGE(STATUS "SYSTEM_KRB5_LIBRARY_DIRS  ${SYSTEM_KRB5_LIBRARY_DIRS}")
    MESSAGE(STATUS "SYSTEM_KRB5_LDFLAGS  ${SYSTEM_KRB5_LDFLAGS}")
    MESSAGE(STATUS "SYSTEM_KRB5_LDFLAGS_OTHER  ${SYSTEM_KRB5_LDFLAGS_OTHER}")
    MESSAGE(STATUS "SYSTEM_KRB5_INCLUDE_DIRS  ${SYSTEM_KRB5_INCLUDE_DIRS}")
    MESSAGE(STATUS "SYSTEM_KRB5_CFLAGS  ${SYSTEM_KRB5_CFLAGS}")
    MESSAGE(STATUS "SYSTEM_KRB5_CFLAGS_OTHER  ${SYSTEM_KRB5_CFLAGS_OTHER}")

    IF(SYSTEM_KRB5_FOUND)
      SET(KERBEROS_LIBRARIES "${SYSTEM_KRB5_LDFLAGS}")
    ELSE()
      # Oracle Linux 6
      FIND_PROGRAM(MY_KRB5_CONFIG krb5-config)
      IF(MY_KRB5_CONFIG)
        EXECUTE_PROCESS(COMMAND ${MY_KRB5_CONFIG} --libs
          OUTPUT_VARIABLE MY_KRB5_LIBS
          OUTPUT_STRIP_TRAILING_WHITESPACE
          RESULT_VARIABLE MY_KRB5_RESULT
          )
        MESSAGE(STATUS "${MY_KRB5_CONFIG} --libs: ${MY_KRB5_LIBS}")
        STRING(REPLACE " " ";" MY_KRB5_LIBS "${MY_KRB5_LIBS}")
        SET(KERBEROS_LIBRARIES "${MY_KRB5_LIBS}")
      ENDIF()
    ENDIF()
  ELSEIF(SOLARIS OR APPLE)
    # Solaris: /usr/lib/64/libkrb5.so
    # Apple: /usr/lib/libkrb5.dylib   which depends on /System/..../Heimdal
    FIND_LIBRARY(KERBEROS_SYSTEM_LIBRARY NAMES "krb5")
    IF(KERBEROS_SYSTEM_LIBRARY)
      SET(KERBEROS_LIBRARIES "${KERBEROS_SYSTEM_LIBRARY}")
    ENDIF()
  ENDIF()

  IF(KERBEROS_LIBRARIES)
    CMAKE_PUSH_CHECK_STATE()

    IF(SOLARIS)
      SET(CMAKE_REQUIRED_INCLUDES "/usr/include/kerberosv5")
    ELSEIF(FREEBSD)
      SET(CMAKE_REQUIRED_INCLUDES "${SYSTEM_KRB5_INCLUDE_DIRS}")
    ELSEIF(LINUX_DEBIAN OR LINUX_UBUNTU)
      SET(CMAKE_REQUIRED_INCLUDES "${SYSTEM_KRB5_CFLAGS}")
    ENDIF()

    CHECK_INCLUDE_FILE(krb5/krb5.h HAVE_KRB5_KRB5_H)
    IF(HAVE_KRB5_KRB5_H)
      FIND_PATH(KERBEROS_INCLUDE_DIR
        NAMES "krb5/krb5.h"
        HINTS ${CMAKE_REQUIRED_INCLUDES}
        )
    ENDIF()
    CMAKE_POP_CHECK_STATE()
  ENDIF()

ENDMACRO()

# Lookup and copy misc libraries, 'objdump -p xx | grep NEED' shows:
# libkrb5.so depends on:
#   NEEDED               libk5crypto.so.3
#   NEEDED               libcom_err.so.3
#   NEEDED               libkrb5support.so.0
# libsasl2.so depends on:
#   NEEDED               libgssapi_krb5.so.2
#   NEEDED               libkrb5.so.3
#   NEEDED               libk5crypto.so.3
#   NEEDED               libcom_err.so.2
SET(CUSTOM_KERBEROS_EXTRA_LIBRARIES
  com_err
  gssapi_krb5
  k5crypto
  krb5support
  )

MACRO(FIND_CUSTOM_KERBEROS)
  # Header file first search in WITH_KERBEROS.
  FIND_PATH(KERBEROS_ROOT_DIR
    NAMES include/krb5/krb5.h
    NO_CMAKE_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    HINTS ${WITH_KERBEROS_PATH}
  )
  # Then search in standard places (if not found above).
  FIND_PATH(KERBEROS_ROOT_DIR
    NAMES include/krb5/krb5.h
    )

  FIND_PATH(KERBEROS_INCLUDE_DIR
    NAMES krb5/krb5.h
    HINTS ${KERBEROS_ROOT_DIR}/include
    )

  FIND_LIBRARY(KERBEROS_CUSTOM_LIBRARY
    NAMES "krb5"
    PATHS ${WITH_KERBEROS}/lib
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    )

  FOREACH(EXTRA_LIB ${CUSTOM_KERBEROS_EXTRA_LIBRARIES})
    SET(VAR_NAME "KERBEROS_CUSTOM_LIBRARY_${EXTRA_LIB}")
    FIND_LIBRARY(${VAR_NAME}
      NAMES "${EXTRA_LIB}"
      PATHS ${WITH_KERBEROS}/lib
      NO_DEFAULT_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      )
  ENDFOREACH()

  IF(KERBEROS_INCLUDE_DIR AND KERBEROS_CUSTOM_LIBRARY)
    MESSAGE(STATUS "KERBEROS_INCLUDE_DIR ${KERBEROS_INCLUDE_DIR}")
    SET(KERBEROS_FOUND 1)
    SET(HAVE_KRB5_KRB5_H 1 CACHE INTERNAL "")
  ENDIF()

ENDMACRO()

MACRO(MYSQL_CHECK_KERBEROS)
  # No Kerberos support for Windows.
  IF(WIN32)
    SET(WITH_KERBEROS "none")
    SET(WITH_KERBEROS "none" CACHE INTERNAL "")
  ENDIF()

  IF(WITH_AUTHENTICATION_LDAP AND (LINUX_RPM OR LINUX_DEB))
    SET(WITH_KERBEROS "system" CACHE STRING "${WITH_KERBEROS_DOC_STRING}" FORCE)
  ELSE()
    RESET_KERBEROS_VARIABLES()
    SET(WITH_KERBEROS "none" CACHE STRING "${WITH_KERBEROS_DOC_STRING}" FORCE)
  ENDIF()

  IF(NOT WITH_KERBEROS)
    IF(WITH_AUTHENTICATION_LDAP)
      SET(WITH_KERBEROS "system" CACHE STRING "${WITH_KERBEROS_DOC_STRING}")
    ELSE()
      SET(WITH_KERBEROS "none" CACHE STRING "${WITH_KERBEROS_DOC_STRING}")
    ENDIF()
  ENDIF()

  # See if WITH_KERBEROS is of the form </path/to/custom/installation>
  FILE(GLOB WITH_KERBEROS_HEADER ${WITH_KERBEROS}/include/krb5/krb5.h)
  IF(WITH_KERBEROS_HEADER)
    FILE(TO_CMAKE_PATH "${WITH_KERBEROS}" WITH_KERBEROS)
    SET(WITH_KERBEROS_PATH ${WITH_KERBEROS})
  ENDIF()

  IF(WITH_KERBEROS STREQUAL "system")
    FIND_SYSTEM_KERBEROS()
  ELSEIF(WITH_KERBEROS STREQUAL "none")
    MESSAGE(STATUS "KERBEROS path is none, disabling kerberos support.")
    SET(WITH_KERBEROS 0)
    SET(KERBEROS_FOUND 0)
  ELSEIF(WITH_KERBEROS_PATH)
    IF(LINUX_STANDALONE)
      FIND_CUSTOM_KERBEROS()
    ELSE()
      MESSAGE(FATAL_ERROR
        "-DWITH_KERBEROS=<path> not supported on this platform")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "Could not find KERBEROS")
  ENDIF()

  IF((KERBEROS_LIBRARIES OR KERBEROS_CUSTOM_LIBRARY) AND HAVE_KRB5_KRB5_H)
    SET(KERBEROS_FOUND 1)
  ELSE()
    SET(KERBEROS_FOUND 0)
    # TODO: FATAL_ERROR later if WITH_AUTHENTICATION_LDAP == ON
    IF(WITH_KERBEROS)
      MESSAGE(WARNING "Could not find KERBEROS")
    ENDIF()
  ENDIF()

  IF(KERBEROS_FOUND)
    SET(KERBEROS_LIB_CONFIGURED 1)
  ENDIF()

ENDMACRO()

MACRO(MYSQL_CHECK_KERBEROS_DLLS)
  IF(LINUX_STANDALONE AND KERBEROS_CUSTOM_LIBRARY)
    COPY_CUSTOM_SHARED_LIBRARY("${KERBEROS_CUSTOM_LIBRARY}" ""
      KERBEROS_LIBRARIES kerberos_target
      )
    FOREACH(EXTRA_LIB ${CUSTOM_KERBEROS_EXTRA_LIBRARIES})
      SET(COPIED_LIBRARY_NAME)
      SET(COPIED_TARGET_NAME)
      SET(VAR_NAME "KERBEROS_CUSTOM_LIBRARY_${EXTRA_LIB}")
      COPY_CUSTOM_SHARED_LIBRARY("${${VAR_NAME}}" ""
        COPIED_LIBRARY_NAME COPIED_TARGET_NAME
        )
      ADD_DEPENDENCIES(${kerberos_target} ${COPIED_TARGET_NAME})
      # Append all which are NEEDED by libkrb5.so
      IF(NOT COPIED_LIBRARY_NAME MATCHES "gssapi")
        LIST(APPEND KERBEROS_LIBRARIES "${COPIED_LIBRARY_NAME}")
      ENDIF()
    ENDFOREACH()
  ENDIF()
ENDMACRO()
