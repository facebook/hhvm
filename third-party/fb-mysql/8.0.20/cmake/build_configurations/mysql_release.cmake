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

# This file includes build settings used for MySQL release

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)

OPTION(DEBUG_EXTNAME "" ON)

IF(LINUX AND CMAKE_BUILD_TYPE)
  IF(CMAKE_BUILD_TYPE_UPPER MATCHES "REL")
    OPTION(REPRODUCIBLE_BUILD "" ON)
  ENDIF()
ENDIF()

IF(NOT COMPILATION_COMMENT)
  SET(COMPILATION_COMMENT "MySQL Community (GPL)")
ENDIF()

IF(NOT COMPILATION_COMMENT_SERVER)
  SET(COMPILATION_COMMENT_SERVER "MySQL Community Server (GPL)")
ENDIF()

IF(LINUX)
  IF(NOT IGNORE_AIO_CHECK)
    # Ensure aio is available on Linux (required by InnoDB)
    CHECK_INCLUDE_FILES(libaio.h HAVE_LIBAIO_H)
    CHECK_LIBRARY_EXISTS(aio io_queue_init "" HAVE_LIBAIO)
    IF(NOT HAVE_LIBAIO_H OR NOT HAVE_LIBAIO)
      MESSAGE(FATAL_ERROR "
        aio is required on Linux, you need to install the required library:

          Debian/Ubuntu:              apt-get install libaio-dev
          RedHat/Fedora/Oracle Linux: yum install libaio-devel
          SuSE:                       zypper install libaio-devel

        If you really do not want it, pass -DIGNORE_AIO_CHECK to cmake.
        ")
    ENDIF()
  ENDIF()
ENDIF()
