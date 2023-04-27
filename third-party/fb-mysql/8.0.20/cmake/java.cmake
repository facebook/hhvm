# Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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

# In PB look in /usr/local/java/jdk1.8-64 even if ENV{JAVA_HOME} is unset
# TODO: integrate with storage/ndb/CMakeLists.txt
FIND_PACKAGE(Java COMPONENTS Runtime)
IF(NOT JAVA_FOUND)
  IF(DEFINED ENV{JAVA_HOME})
    # Could not find Java in the specific location set by JAVA_HOME
    # or in standard paths, don't search further
    MESSAGE(FATAL_ERROR "Could NOT find Java: neither in specified "
      "JAVA_HOME=" $ENV{JAVA_HOME} " or standard location")
  ENDIF()
  # Prefer Java with same bit size as current build
  SET(_bit_suffix "-64")

  # Use well known standard base
  SET(_base_path /usr/local/java/)

  # Search for version in specified order
  SET(_preferred_versions
    1.8
    1.7
    1.6)

  FOREACH(_version ${_preferred_versions})
    SET(_path ${_base_path}jdk${_version}${_bit_suffix})
    MESSAGE(STATUS "Looking for Java in ${_path}...")
    SET(ENV{JAVA_HOME} ${_path})
    FIND_PACKAGE(Java ${_version} COMPONENTS Runtime)
    IF(JAVA_FOUND)
      # Found java, no need to search further
      MESSAGE(STATUS "Found Java in ${_path}")
      BREAK()
    ENDIF()
  ENDFOREACH()

  IF(NOT JAVA_FOUND)
    # Could not find Java in well known locations either
    MESSAGE(FATAL_ERROR "Could NOT find suitable version of Java")
  ENDIF()
ENDIF()

GET_FILENAME_COMPONENT(Java_JAVA_bin ${Java_JAVA_EXECUTABLE} PATH)
MESSAGE(STATUS "Java_JAVA_EXECUTABLE is ${Java_JAVA_EXECUTABLE}")
MESSAGE(STATUS "Java_JAVA_bin is ${Java_JAVA_bin}")
