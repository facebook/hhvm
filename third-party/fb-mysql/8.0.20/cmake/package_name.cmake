# Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.
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

# Produce meaningful package name for the binary package
# The logic is rather involved with special cases for  different OSes

MACRO(GET_PACKAGE_FILE_NAME Var)
  IF(NOT VERSION)
    MESSAGE(FATAL_ERROR
     "Variable VERSION needs to be set prior to calling GET_PACKAGE_FILE_NAME")
  ENDIF()
  IF(NOT SYSTEM_NAME_AND_PROCESSOR)
    SET(NEED_DASH_BETWEEN_PLATFORM_AND_MACHINE 1)
    SET(DEFAULT_PLATFORM ${CMAKE_SYSTEM_NAME})
    SET(DEFAULT_MACHINE  ${CMAKE_SYSTEM_PROCESSOR})
    IF(SIZEOF_VOIDP EQUAL 8)
      SET(64BIT 1)
    ENDIF()

    IF(WIN32)
      SET(NEED_DASH_BETWEEN_PLATFORM_AND_MACHINE 0)
      SET(DEFAULT_PLATFORM "win")
      IF(64BIT)
        SET(DEFAULT_MACHINE "x64")
      ELSE()
        SET(DEFAULT_MACHINE "32")
      ENDIF()
    ELSEIF(LINUX)
      IF(NOT 64BIT AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        SET(DEFAULT_MACHINE "i686")
      ENDIF()
    ELSEIF(SOLARIS)
      # SunOS 5.10=> solaris10
      STRING(REPLACE "5." "" VER "${CMAKE_SYSTEM_VERSION}")
      SET(DEFAULT_PLATFORM "solaris${VER}")
      IF(64BIT)
        IF(SOLARIS_INTEL)
          SET(DEFAULT_MACHINE "x86_64")
        ELSE()
          SET(DEFAULT_MACHINE "${CMAKE_SYSTEM_PROCESSOR}-64bit")
        ENDIF()
      ENDIF()
    ELSEIF(FREEBSD)
      STRING(REGEX MATCH "[0-9]+\\.[0-9]+"  VER "${CMAKE_SYSTEM_VERSION}")
      SET(DEFAULT_PLATFORM "${CMAKE_SYSTEM_NAME}${VER}")
      IF(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
        SET(DEFAULT_MACHINE "x86_64")
        IF(NOT 64BIT)
          SET(DEFAULT_MACHINE "i386")
        ENDIF()
      ENDIF()
    ELSEIF(APPLE)
      IF(CMAKE_OSX_DEPLOYMENT_TARGET)
        SET(DEFAULT_PLATFORM "osx${CMAKE_OSX_DEPLOYMENT_TARGET}")
      ELSE()
        SET(VER "${CMAKE_SYSTEM_VERSION}")
        STRING(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" VER "${VER}")
        # Subtract 4 from Darwin version to get correct osx10.X
        MATH(EXPR VER  "${VER} -4")
        SET(DEFAULT_PLATFORM "osx10.${VER}")
      ENDIF()

      IF(CMAKE_OSX_ARCHITECTURES)
        LIST(LENGTH CMAKE_OSX_ARCHITECTURES LEN)
        IF(LEN GREATER 1)
          SET(DEFAULT_MACHINE "universal")
        ELSE()
          SET(DEFAULT_MACHINE "${CMAKE_OSX_ARCHITECTURES}")
        ENDIF()
      ELSE()
        IF(64BIT)
          SET(DEFAULT_MACHINE "x86_64")
        ENDIF()
      ENDIF()

      IF(DEFAULT_MACHINE MATCHES "i386")
        SET(DEFAULT_MACHINE "x86")
      ENDIF()
    ENDIF()

    IF(NOT PLATFORM)
      SET(PLATFORM ${DEFAULT_PLATFORM})
    ENDIF()
    IF(NOT MACHINE)
      SET(MACHINE ${DEFAULT_MACHINE})
    ENDIF()

    IF(NEED_DASH_BETWEEN_PLATFORM_AND_MACHINE)
      SET(SYSTEM_NAME_AND_PROCESSOR "${PLATFORM}-${MACHINE}")
    ELSE()
      SET(SYSTEM_NAME_AND_PROCESSOR "${PLATFORM}${MACHINE}")
    ENDIF()
  ENDIF()

  IF(SHORT_PRODUCT_TAG)
    SET(PRODUCT_TAG "-${SHORT_PRODUCT_TAG}")
  ELSEIF(MYSQL_SERVER_SUFFIX)
    SET(PRODUCT_TAG "${MYSQL_SERVER_SUFFIX}")  # Already has a leading dash
  ELSE()
    SET(PRODUCT_TAG)
  ENDIF()

  IF(WITH_NDBCLUSTER)
    SET(package_name "mysql${PRODUCT_TAG}-${MYSQL_CLUSTER_VERSION}-${SYSTEM_NAME_AND_PROCESSOR}")
  ELSE()
    SET(package_name "mysql${PRODUCT_TAG}-${VERSION}-${SYSTEM_NAME_AND_PROCESSOR}")
  ENDIF()

  MESSAGE(STATUS "Packaging as: ${package_name}")

  # Sometimes package suffix is added (something like "-icc-glibc23")
  IF(PACKAGE_SUFFIX)
    SET(package_name "${package_name}${PACKAGE_SUFFIX}")
  ENDIF()
  STRING(TOLOWER ${package_name} package_name)
  SET(${Var} ${package_name})
ENDMACRO()


IF(NOT CPACK_PACKAGE_FILE_NAME)
  GET_PACKAGE_FILE_NAME(CPACK_PACKAGE_FILE_NAME)
ENDIF()

IF(NOT CPACK_SOURCE_PACKAGE_FILE_NAME)
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "mysql-${VERSION}")
  IF(WITH_NDBCLUSTER)
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "mysql-cluster-gpl-${MYSQL_CLUSTER_VERSION}")
    MESSAGE(STATUS "MySQL Cluster package name: ${CPACK_SOURCE_PACKAGE_FILE_NAME}")
  ELSE()
  ENDIF()
ENDIF()
SET(CPACK_PACKAGE_CONTACT "MySQL Release Engineering <mysql-build@oss.oracle.com>")
SET(CPACK_PACKAGE_VENDOR "Oracle Corporation")
SET(CPACK_SOURCE_GENERATOR "TGZ")
INCLUDE(cpack_source_ignore_files)

# Defintions for windows version resources
SET(PRODUCTNAME "MySQL Server")
SET(COMPANYNAME ${CPACK_PACKAGE_VENDOR})

STRING(TIMESTAMP MYSQL_COPYRIGHT_YEAR "%Y")

# Add version information to the exe and dll files
# Refer to http://msdn.microsoft.com/en-us/library/aa381058(VS.85).aspx
# for more info.
IF(MSVC)
  GET_FILENAME_COMPONENT(MYSQL_CMAKE_SCRIPT_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

  SET(FILETYPE VFT_APP)
  CONFIGURE_FILE(${MYSQL_CMAKE_SCRIPT_DIR}/versioninfo.rc.in
    ${CMAKE_BINARY_DIR}/versioninfo_exe.rc)

  SET(FILETYPE VFT_DLL)
  CONFIGURE_FILE(${MYSQL_CMAKE_SCRIPT_DIR}/versioninfo.rc.in
    ${CMAKE_BINARY_DIR}/versioninfo_dll.rc)

  FUNCTION(ADD_VERSION_INFO target target_type sources_var)
    IF("${target_type}" MATCHES "SHARED" OR "${target_type}" MATCHES "MODULE")
      SET(rcfile ${CMAKE_BINARY_DIR}/versioninfo_dll.rc)
    ELSEIF("${target_type}" MATCHES "EXE")
      SET(rcfile ${CMAKE_BINARY_DIR}/versioninfo_exe.rc)
    ENDIF()
    SET(${sources_var} ${${sources_var}} ${rcfile} PARENT_SCOPE)
  ENDFUNCTION()

ELSE()
  FUNCTION(ADD_VERSION_INFO)
  ENDFUNCTION()
ENDIF()
