# Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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

IF(WIN32)
  RETURN()
ENDIF()

MACRO(MYSQL_CHECK_RPC)
  IF(LINUX AND NOT LIBTIRPC_VERSION_TOO_OLD)
    MYSQL_CHECK_PKGCONFIG()
    PKG_CHECK_MODULES(TIRPC libtirpc)
  ENDIF()

  IF(TIRPC_FOUND)
    IF(TIRPC_VERSION VERSION_LESS 1.0)
      SET(LIBTIRPC_VERSION_TOO_OLD 1 CACHE INTERNAL "libtirpc is too old" FORCE)
      MESSAGE(WARNING
        "Ignoring libtirpc version ${TIRPC_VERSION}, need at least 1.0")
      UNSET(TIRPC_FOUND)
      UNSET(TIRPC_FOUND CACHE)
      GET_CMAKE_PROPERTY(CACHE_VARS CACHE_VARIABLES)
      FOREACH(CACHE_VAR ${CACHE_VARS})
        IF(CACHE_VAR MATCHES "^TIRPC_.*")
          UNSET(${CACHE_VAR})
          UNSET(${CACHE_VAR} CACHE)
        ENDIF()
      ENDFOREACH()
    ENDIF()
  ENDIF()

  IF(TIRPC_FOUND)
    ADD_DEFINITIONS(-DHAVE_TIRPC)

    # RPC headers may be found in /usr/include rather than /usr/include/tirpc
    IF(TIRPC_INCLUDE_DIRS)
      SET(RPC_INCLUDE_DIRS ${TIRPC_INCLUDE_DIRS})
      INCLUDE_DIRECTORIES(SYSTEM "${TIRPC_INCLUDE_DIRS}")
    ELSE()
      FIND_PATH(RPC_INCLUDE_DIRS NAMES rpc/rpc.h)
    ENDIF()
  ELSE()
    FIND_PATH(RPC_INCLUDE_DIRS NAMES rpc/rpc.h)
  ENDIF()

  IF(NOT RPC_INCLUDE_DIRS)
    MESSAGE(FATAL_ERROR
      "Could not find rpc/rpc.h in /usr/include or /usr/include/tirpc")
  ENDIF()

ENDMACRO()
