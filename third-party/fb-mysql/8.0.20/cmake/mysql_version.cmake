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
# Global constants, only to be changed between major releases.
#

SET(SHARED_LIB_MAJOR_VERSION "21")
SET(SHARED_LIB_MINOR_VERSION "1")
SET(PROTOCOL_VERSION "10")

# Generate "something" to trigger cmake rerun when VERSION changes
CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/VERSION
  ${CMAKE_BINARY_DIR}/VERSION.dep
)

# Read value for a variable from VERSION.

MACRO(MYSQL_GET_CONFIG_VALUE keyword var)
 IF(NOT ${var})
   FILE (STRINGS ${CMAKE_SOURCE_DIR}/VERSION str REGEX "^[ ]*${keyword}=")
   IF(str)
     STRING(REPLACE "${keyword}=" "" str ${str})
     STRING(REGEX REPLACE  "[ ].*" ""  str "${str}")
     SET(${var} ${str})
   ENDIF()
 ENDIF()
ENDMACRO()


# Read mysql version for configure script

MACRO(GET_MYSQL_VERSION)
  MYSQL_GET_CONFIG_VALUE("MYSQL_VERSION_MAJOR" MAJOR_VERSION)
  MYSQL_GET_CONFIG_VALUE("MYSQL_VERSION_MINOR" MINOR_VERSION)
  MYSQL_GET_CONFIG_VALUE("MYSQL_VERSION_PATCH" PATCH_VERSION)
  MYSQL_GET_CONFIG_VALUE("MYSQL_VERSION_EXTRA" EXTRA_VERSION)

  IF(NOT DEFINED MAJOR_VERSION OR
     NOT DEFINED MINOR_VERSION OR
     NOT DEFINED PATCH_VERSION)
    MESSAGE(FATAL_ERROR "VERSION file cannot be parsed.")
  ENDIF()

  SET(VERSION
    "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}${EXTRA_VERSION}")
  MESSAGE(STATUS "MySQL ${VERSION}")
  SET(MYSQL_BASE_VERSION
    "${MAJOR_VERSION}.${MINOR_VERSION}" CACHE INTERNAL "MySQL Base version")
  SET(MYSQL_NO_DASH_VERSION
    "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
  STRING(REGEX REPLACE "^-" "." MYSQL_VERSION_EXTRA_DOT "${EXTRA_VERSION}")
  SET(VERSION_SRC "${VERSION}")

  MATH(EXPR MYSQL_VERSION_ID
    "10000*${MAJOR_VERSION} + 100*${MINOR_VERSION} + ${PATCH_VERSION}")
  MARK_AS_ADVANCED(VERSION MYSQL_VERSION_ID MYSQL_BASE_VERSION)
  SET(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
  SET(CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
  SET(CPACK_PACKAGE_VERSION_PATCH ${PATCH_VERSION})

  IF(WITH_NDBCLUSTER)
    # Read MySQL Cluster version values from VERSION, these are optional
    # as by default MySQL Cluster is using the MySQL Server version
    MYSQL_GET_CONFIG_VALUE("MYSQL_CLUSTER_VERSION_MAJOR" CLUSTER_MAJOR_VERSION)
    MYSQL_GET_CONFIG_VALUE("MYSQL_CLUSTER_VERSION_MINOR" CLUSTER_MINOR_VERSION)
    MYSQL_GET_CONFIG_VALUE("MYSQL_CLUSTER_VERSION_PATCH" CLUSTER_PATCH_VERSION)
    MYSQL_GET_CONFIG_VALUE("MYSQL_CLUSTER_VERSION_EXTRA" CLUSTER_EXTRA_VERSION)

    # Set MySQL Cluster version same as the MySQL Server version
    # unless a specific MySQL Cluster version has been specified
    # in the VERSION file. This is the version used when creating
    # the cluster package names as well as by all the NDB binaries.
    IF(DEFINED CLUSTER_MAJOR_VERSION AND
       DEFINED CLUSTER_MINOR_VERSION AND
       DEFINED CLUSTER_PATCH_VERSION)
      # Set MySQL Cluster version to the specific version defined in VERSION
      SET(MYSQL_CLUSTER_VERSION "${CLUSTER_MAJOR_VERSION}")
      SET(MYSQL_CLUSTER_VERSION
        "${MYSQL_CLUSTER_VERSION}.${CLUSTER_MINOR_VERSION}")
      SET(MYSQL_CLUSTER_VERSION
        "${MYSQL_CLUSTER_VERSION}.${CLUSTER_PATCH_VERSION}")
      IF(DEFINED CLUSTER_EXTRA_VERSION)
        SET(MYSQL_CLUSTER_VERSION
          "${MYSQL_CLUSTER_VERSION}${CLUSTER_EXTRA_VERSION}")
      ENDIF()
    ELSE()
      # Set MySQL Cluster version to the same as MySQL Server, possibly
      # overriding the extra version with value specified in VERSION
      # This might be used when MySQL Cluster is still released as DMR
      # while MySQL Server is already GA.
      SET(MYSQL_CLUSTER_VERSION
        "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
      IF(DEFINED CLUSTER_EXTRA_VERSION)
        # Using specific MySQL Cluster extra version
        SET(MYSQL_CLUSTER_VERSION
          "${MYSQL_CLUSTER_VERSION}${CLUSTER_EXTRA_VERSION}")
        # Override the extra version for rpm packages
        STRING(REGEX REPLACE "^-" "." MYSQL_VERSION_EXTRA_DOT
          "${CLUSTER_EXTRA_VERSION}")
      ELSE()
        SET(MYSQL_CLUSTER_VERSION
          "${MYSQL_CLUSTER_VERSION}${EXTRA_VERSION}")
      ENDIF()
    ENDIF()
    MESSAGE(STATUS "MySQL Cluster version: ${MYSQL_CLUSTER_VERSION}")

    SET(VERSION_SRC "${MYSQL_CLUSTER_VERSION}")

    # Set suffix to indicate that this is MySQL Server built for MySQL Cluster
    SET(MYSQL_SERVER_SUFFIX "-cluster")
  ENDIF()
ENDMACRO()

# Get mysql version and other interesting variables
GET_MYSQL_VERSION()

SET(SHARED_LIB_PATCH_VERSION ${PATCH_VERSION})

SET(MYSQL_TCP_PORT_DEFAULT "3306")

IF(NOT MYSQL_TCP_PORT)
  SET(MYSQL_TCP_PORT ${MYSQL_TCP_PORT_DEFAULT})
  SET(MYSQL_TCP_PORT_DEFAULT "0")
ELSEIF(MYSQL_TCP_PORT EQUAL MYSQL_TCP_PORT_DEFAULT)
  SET(MYSQL_TCP_PORT_DEFAULT "0")
ENDIF()

IF(NOT MYSQL_ADMIN_TCP_PORT)
  SET(MYSQL_ADMIN_TCP_PORT 33062)
ENDIF(NOT MYSQL_ADMIN_TCP_PORT)

IF(NOT MYSQL_UNIX_ADDR)
  SET(MYSQL_UNIX_ADDR "/tmp/mysql.sock")
ENDIF()
IF(NOT COMPILATION_COMMENT)
  SET(COMPILATION_COMMENT "Source distribution")
ENDIF()
IF(NOT COMPILATION_COMMENT_SERVER)
  SET(COMPILATION_COMMENT_SERVER ${COMPILATION_COMMENT})
ENDIF()
