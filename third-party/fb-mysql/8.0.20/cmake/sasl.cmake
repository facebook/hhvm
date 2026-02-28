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

# cmake -DWITH_SASL=system|</path/to/custom/installation>
# system is the default
# Custom path is only supported for LINUX_STANDALONE and WIN32 platforms.

INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFiles)

SET(WITH_SASL_DOC "\nsystem (use the OS sasl library)")
STRING_APPEND(WITH_SASL_DOC ", \n</path/to/custom/installation>")

STRING(REPLACE "\n" "| " WITH_SASL_DOC_STRING "${WITH_SASL_DOC}")

MACRO(RESET_SASL_VARIABLES)
  UNSET(SASL_INCLUDE_DIR)
  UNSET(SASL_INCLUDE_DIR CACHE)
  UNSET(SASL_LIBRARY)
  UNSET(SASL_LIBRARY CACHE)
  UNSET(SASL_ROOT_DIR)
  UNSET(SASL_ROOT_DIR CACHE)
  UNSET(WITH_SASL)
  UNSET(WITH_SASL CACHE)
  UNSET(WITH_SASL_PATH)
  UNSET(WITH_SASL_PATH CACHE)
  UNSET(HAVE_CUSTOM_SASL_SASL_H)
  UNSET(HAVE_CUSTOM_SASL_SASL_H CACHE)
  UNSET(HAVE_SASL_SASL_H)
  UNSET(HAVE_SASL_SASL_H CACHE)
ENDMACRO()

FUNCTION(WARN_MISSING_SYSTEM_SASL OUTPUT_WARNING)
  # The SASL scram plugin is not needed for build, but it is needed for testing.
  IF(SASL_SYSTEM_LIBRARY)
    IF(SASL_SYSTEM_SCRAM_LIBRARY)
      RETURN()
    ELSEIF(SASL_VERSION VERSION_LESS "2.1.25")
      MESSAGE(WARNING "SASL version is ${SASL_VERSION} SCRAM not supported")
      RETURN()
    ENDIF()
  ENDIF()
  IF(WITH_SASL STREQUAL "system")
    SET(DEBIAN_PKGS "libsasl2-dev libsasl2-modules-gssapi-mit")
    SET(REDHAT_PKGS "cyrus-sasl-devel cyrus-sasl-scram")
    SET(SUSE_PKGS "cyrus-sasl-devel cyrus-sasl-scram")

    MESSAGE(WARNING "Cannot find SASL development libraries. "
      "You need to install the required packages:\n"
      "  Debian/Ubuntu:              apt install ${DEBIAN_PKGS}\n"
      "  RedHat/Fedora/Oracle Linux: yum install ${REDHAT_PKGS}\n"
      "  SuSE:                       zypper install ${SUSE_PKGS}\n"
      )
    SET(${OUTPUT_WARNING} 1 PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()

MACRO(FIND_SASL_VERSION)
  IF(SASL_INCLUDE_DIR AND EXISTS "${SASL_INCLUDE_DIR}/sasl/sasl.h")
    FOREACH(version_part
        SASL_VERSION_MAJOR
        SASL_VERSION_MINOR
        SASL_VERSION_STEP)
      # Find the #define and extract the number.
      FILE(STRINGS "${SASL_INCLUDE_DIR}/sasl/sasl.h" ${version_part}
        REGEX "^#[\t ]*define[\t ]+${version_part}[\t ]+([0-9]+).*")
      STRING(REGEX REPLACE
        "^.*${version_part}[\t ]+([0-9]+).*" "\\1"
        ${version_part} "${${version_part}}")
    ENDFOREACH()
    SET(SASL_VERSION
      "${SASL_VERSION_MAJOR}.${SASL_VERSION_MINOR}.${SASL_VERSION_STEP}")
    SET(SASL_VERSION "${SASL_VERSION}" CACHE INTERNAL "SASL major.minor.step")
    MESSAGE(STATUS "SASL_VERSION ${SASL_VERSION}")
  ENDIF()
ENDMACRO()

MACRO(FIND_SYSTEM_SASL)
  # Cyrus SASL 2.1.26 on Solaris 11.4 has a bug that requires sys/types.h
  # to be included before checking if sasl/sasl.h exists
  CHECK_INCLUDE_FILES("sys/types.h;sasl/sasl.h" HAVE_SASL_SASL_H)
  IF(HAVE_SASL_SASL_H)
    FIND_PATH(SASL_INCLUDE_DIR NAMES "sasl/sasl.h")
  ENDIF()
  FIND_LIBRARY(SASL_SYSTEM_LIBRARY NAMES "sasl2" "sasl")
  FIND_LIBRARY(SASL_SYSTEM_SCRAM_LIBRARY NAMES scram PATH_SUFFIXES sasl2)
  IF (SASL_SYSTEM_LIBRARY)
    SET(SASL_LIBRARY ${SASL_SYSTEM_LIBRARY})
    MESSAGE(STATUS "SASL_LIBRARY ${SASL_LIBRARY}")
  ENDIF()
ENDMACRO()

# Lookup and copy all plugins
#   lib/sasl2/libanonymous.so
#   lib/sasl2/libcrammd5.so
#   lib/sasl2/libdigestmd5.so
#   lib/sasl2/libgs2.so
#   lib/sasl2/libgssapiv2.so
#   lib/sasl2/libplain.so
#   lib/sasl2/libscram.so
SET(CUSTOM_SASL_PLUGINS
  anonymous
  crammd5
  digestmd5
  gs2
  gssapiv2
  plain
  scram
  )

MACRO(FIND_CUSTOM_SASL)
  # First search in WITH_SASL_PATH.
  FIND_PATH(SASL_ROOT_DIR
    NAMES include/sasl/sasl.h
    NO_CMAKE_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    HINTS ${WITH_SASL_PATH}
    )
  # Then search in standard places (if not found above).
  FIND_PATH(SASL_ROOT_DIR
    NAMES include/sasl/sasl.h
    )

  FIND_PATH(SASL_INCLUDE_DIR
    NAMES sasl/sasl.h
    HINTS ${SASL_ROOT_DIR}/include
    )

  # The static library has all the plugins built-in.
  LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
  FIND_LIBRARY(SASL_STATIC_LIBRARY
    NAMES sasl2 sasl libsasl
    PATHS ${WITH_SASL}/lib
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_SYSTEM_ENVIRONMENT_PATH)
  LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)

  FIND_LIBRARY(SASL_CUSTOM_LIBRARY
    NAMES sasl2 sasl libsasl
    PATHS ${WITH_SASL}/lib
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    )

  IF(SASL_CUSTOM_LIBRARY)
    SET(SASL_LIBRARY ${SASL_CUSTOM_LIBRARY})
    MESSAGE(STATUS "SASL_LIBRARY ${SASL_LIBRARY}")
  ENDIF()

  IF(LINUX_STANDALONE)
    FOREACH(SASL_PLUGIN ${CUSTOM_SASL_PLUGINS})
      SET(VAR_NAME "SASL_CUSTOM_PLUGIN_${SASL_PLUGIN}")
      FIND_LIBRARY(${VAR_NAME}
        NAMES "${SASL_PLUGIN}"
        PATHS ${WITH_SASL}/lib/sasl2
        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        )
      IF(NOT ${VAR_NAME})
        # This should be FATAL_ERROR, enable later
        MESSAGE(WARNING
        "Could not find plugin lib${SASL_PLUGIN}.so in ${WITH_SASL}/lib/sasl2")
      ENDIF()
    ENDFOREACH()
  ENDIF()

  IF(SASL_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(BEFORE SYSTEM "${SASL_INCLUDE_DIR}")

    CMAKE_PUSH_CHECK_STATE()
    SET(CMAKE_REQUIRED_INCLUDES "${SASL_INCLUDE_DIR}")
    # Windows users doing 'git pull' will have cached HAVE_SASL_SASL_H=""
    CHECK_INCLUDE_FILE(sasl/sasl.h HAVE_CUSTOM_SASL_SASL_H)
    IF(HAVE_CUSTOM_SASL_SASL_H)
      SET(HAVE_SASL_SASL_H 1 CACHE INTERNAL "Have include sasl/sasl.h" FORCE)
    ENDIF()
    CMAKE_POP_CHECK_STATE()
  ENDIF()

  IF(WIN32)
    FIND_FILE(SASL_LIBRARY_DLL
      NAMES libsasl.dll
      PATHS ${WITH_SASL}/lib
      NO_CMAKE_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      )
    FIND_FILE(SASL_SCRAM_PLUGIN
      NAMES saslSCRAM.dll
      PATHS ${WITH_SASL}/lib
      NO_CMAKE_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      )
  ENDIF()
ENDMACRO()

MACRO(MYSQL_CHECK_SASL)
  IF(NOT WITH_SASL)
    SET(WITH_SASL "system" CACHE STRING "${WITH_SASL_DOC_STRING}" FORCE)
  ENDIF()

  # See if WITH_SASL is of the form </path/to/custom/installation>
  FILE(GLOB WITH_SASL_HEADER ${WITH_SASL}/include/sasl/sasl.h)
  IF (WITH_SASL_HEADER)
    FILE(TO_CMAKE_PATH "${WITH_SASL}" WITH_SASL)
    SET(WITH_SASL_PATH ${WITH_SASL})
  ENDIF()

  IF(WITH_SASL STREQUAL "system")
    FIND_SYSTEM_SASL()
  ELSEIF(WITH_SASL_PATH)
    IF(LINUX_STANDALONE OR WIN32)
      FIND_CUSTOM_SASL()
    ELSE()
      MESSAGE(FATAL_ERROR "-DWITH_SASL=<path> not supported on this platform")
    ENDIF()
  ELSE()
    RESET_SASL_VARIABLES()
    MESSAGE(FATAL_ERROR "Could not find SASL")
  ENDIF()

  IF(HAVE_SASL_SASL_H AND SASL_LIBRARY)
    FIND_SASL_VERSION()
    SET(SASL_FOUND TRUE)
  ELSE()
    SET(SASL_FOUND FALSE)
    # FATAL_ERROR later if WITH_AUTHENTICATION_LDAP == ON
    MESSAGE(WARNING "Could not find SASL")
  ENDIF()

ENDMACRO()

MACRO(MYSQL_CHECK_SASL_DLLS)
  IF(LINUX_STANDALONE AND SASL_CUSTOM_LIBRARY)
    FIND_OBJECT_DEPENDENCIES(${SASL_CUSTOM_LIBRARY} SASL_DEPENDS_ON)
    STRING(REPLACE ";" " " SASL_DEPENDS_ON "${SASL_DEPENDS_ON}")
    IF(NOT SASL_DEPENDS_ON MATCHES "libkrb5")
      MESSAGE(WARNING "This SASL library is built without KERBEROS")
      SET(SASL_WITHOUT_KERBEROS 1)
      UNSET(KERBEROS_CUSTOM_LIBRARY CACHE)
      UNSET(KERBEROS_CUSTOM_LIBRARY)
      UNSET(KERBEROS_FOUND CACHE)
      UNSET(KERBEROS_FOUND)
      UNSET(KERBEROS_LIBRARIES CACHE)
      UNSET(KERBEROS_LIBRARIES)
      UNSET(KERBEROS_LIB_CONFIGURED CACHE)
      UNSET(KERBEROS_LIB_CONFIGURED)
      UNSET(KERBEROS_SYSTEM_LIBRARY CACHE)
      UNSET(KERBEROS_SYSTEM_LIBRARY)
      SET(WITH_KERBEROS "none" CACHE STRING "" FORCE)
    ENDIF()

    COPY_CUSTOM_SHARED_LIBRARY("${SASL_CUSTOM_LIBRARY}" ""
      SASL_LIBRARY sasl_target)
    FOREACH(SASL_PLUGIN ${CUSTOM_SASL_PLUGINS})
      SET(COPIED_LIBRARY_NAME)
      SET(COPIED_TARGET_NAME)
      SET(VAR_NAME "SASL_CUSTOM_PLUGIN_${SASL_PLUGIN}")
      IF(${VAR_NAME})
        COPY_CUSTOM_SHARED_LIBRARY("${${VAR_NAME}}" "sasl2"
          COPIED_LIBRARY_NAME COPIED_TARGET_NAME
          )
        ADD_DEPENDENCIES(${sasl_target} ${COPIED_TARGET_NAME})
      ELSEIF(SASL_WITHOUT_KERBEROS)
        # This should be FATAL_ERROR
      ELSE()
        MESSAGE(FATAL_ERROR
        "Could not find plugin lib${SASL_PLUGIN}.so in ${WITH_SASL}/lib/sasl2")
      ENDIF()
    ENDFOREACH()
  ENDIF()
ENDMACRO()
