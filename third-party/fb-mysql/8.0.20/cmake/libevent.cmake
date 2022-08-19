# Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.
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

MACRO (MYSQL_USE_BUNDLED_LIBEVENT)
  SET(WITH_LIBEVENT "bundled" CACHE STRING "Use bundled libevent library")
  SET(LIBEVENT_LIBRARIES  event)
  ## openssl is in the 'libevent.a' static lib
  SET(LIBEVENT_OPENSSL )
  SET(LIBEVENT_INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/extra/libevent/include"
    "${CMAKE_BINARY_DIR}/extra/libevent/include")
  SET(LIBEVENT_FOUND  TRUE)
  ADD_DEFINITIONS("-DHAVE_LIBEVENT2")
  ADD_SUBDIRECTORY(extra/libevent)
ENDMACRO()

# MYSQL_CHECK_LIBEVENT
#
# Provides the following configure options:
# WITH_LIBEVENT_BUNDLED
# If this is set,we use bindled libevent
# If this is not set,search for system libevent. 
# if system libevent is not found, use bundled copy
# LIBEVENT_LIBRARIES, LIBEVENT_INCLUDE_DIRS
# are set after this macro has run

MACRO (MYSQL_CHECK_LIBEVENT)

    IF (NOT WITH_LIBEVENT)
      SET(WITH_LIBEVENT "bundled"  CACHE STRING "By default use bundled libevent on this platform")
    ENDIF()
  
  IF(WITH_LIBEVENT STREQUAL "bundled")
    MYSQL_USE_BUNDLED_LIBEVENT()
  ELSEIF(WITH_LIBEVENT STREQUAL "system" OR WITH_LIBEVENT STREQUAL "yes")
    SET(LIBEVENT_FIND_QUIETLY TRUE)

    IF (NOT LIBEVENT_INCLUDE_PATH)
      SET(LIBEVENT_INCLUDE_PATH /usr/local/include /opt/local/include)
    ENDIF()

    FIND_PATH(LIBEVENT_INCLUDE_DIR event.h PATHS ${LIBEVENT_INCLUDE_PATH})

    IF (NOT LIBEVENT_INCLUDE_DIR)
        MESSAGE(SEND_ERROR "Cannot find appropriate event.h in /usr/local/include or /opt/local/include. Use bundled libevent")
    ENDIF()

    IF (NOT LIBEVENT_LIB_PATHS) 
      SET(LIBEVENT_LIB_PATHS /usr/local/lib /opt/local/lib)
    ENDIF()

    ## libevent.so is historical, use libevent_core.so if found.
    FIND_LIBRARY(LIBEVENT_CORE event_core PATHS ${LIBEVENT_LIB_PATHS})
    FIND_LIBRARY(LIBEVENT_EXTRA event_extra PATHS ${LIBEVENT_LIB_PATHS})

    ## libevent_openssl.so is split out on Linux distros
    FIND_LIBRARY(LIBEVENT_OPENSSL event_openssl PATHS ${LIBEVENT_LIB_PATHS})
    FIND_LIBRARY(LIBEVENT_LIB event PATHS ${LIBEVENT_LIB_PATHS})

    if (NOT LIBEVENT_LIB AND NOT LIBEVENT_CORE)
        MESSAGE(SEND_ERROR "Cannot find appropriate event lib in /usr/local/lib or /opt/local/lib. Use bundled libevent")
    ENDIF()

    IF ((LIBEVENT_LIB OR LIBEVENT_CORE) AND LIBEVENT_INCLUDE_DIR)
      SET(LIBEVENT_FOUND TRUE)
      IF (LIBEVENT_CORE)
        SET(LIBEVENT_LIBS ${LIBEVENT_CORE} ${LIBEVENT_EXTRA})
      ELSE()
        SET(LIBEVENT_LIBS ${LIBEVENT_LIB})
      ENDIF()
    ELSE()
      SET(LIBEVENT_FOUND FALSE)
    ENDIF()

    IF(LIBEVENT_FOUND)
      SET(LIBEVENT_LIBRARIES ${LIBEVENT_LIBS})
      SET(LIBEVENT_INCLUDE_DIRS ${LIBEVENT_INCLUDE_DIR})
      FIND_PATH(LIBEVENT2_INCLUDE_DIR event2 HINTS ${LIBEVENT_INCLUDE_PATH}/event)
      IF (LIBEVENT2_INCLUDE_DIR)
        ADD_DEFINITIONS("-DHAVE_LIBEVENT2")
      ELSE()
        MESSAGE(SEND_ERROR "Found libevent libraries, but can not find appropriate event2/ directory in include paths. Install the development package of libevent 2.x")
      ENDIF()
    ELSE()
      IF(WITH_LIBEVENT STREQUAL "system")
        MESSAGE(SEND_ERROR "Cannot find appropriate system libraries for libevent. Use bundled libevent")
      ENDIF()
      MYSQL_USE_BUNDLED_LIBEVENT()
    ENDIF()

  ENDIF()
ENDMACRO()

# Use pkg-config to find lots of SYSTEM_LIBEVENT related information,
# including SYSTEM_LIBEVENT_VERSION which will be something like "2.1.8-stable".
MACRO(MYSQL_CHECK_LIBEVENT_VERSION)
  IF(LINUX)
    MYSQL_CHECK_PKGCONFIG()
    PKG_CHECK_MODULES(SYSTEM_LIBEVENT libevent)
  ENDIF()
ENDMACRO()
