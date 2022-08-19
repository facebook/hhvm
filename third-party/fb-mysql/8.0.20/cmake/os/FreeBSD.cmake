# Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.
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

# This file includes FreeBSD specific options and quirks,
# related to system checks

INCLUDE(CheckCSourceRuns)

SET(FREEBSD 1)

# On FreeBSD some includes, e.g. sasl/sasl.h, is in /usr/local/include
LIST(APPEND CMAKE_REQUIRED_INCLUDES "/usr/local/include")
# Do not INCLUDE_DIRECTORIES here, we need to do that *after* configuring boost,
# in order to search include/boost_1_70_0/patches
# INCLUDE_DIRECTORIES(SYSTEM /usr/local/include)

# We require at least Clang 6.0 (FreeBSD 12).
IF(NOT FORCE_UNSUPPORTED_COMPILER)
  IF(MY_COMPILER_IS_CLANG)
    CHECK_C_SOURCE_RUNS("
      int main()
      {
        return (__clang_major__ < 6);
      }" HAVE_SUPPORTED_CLANG_VERSION)
    IF(NOT HAVE_SUPPORTED_CLANG_VERSION)
      MESSAGE(FATAL_ERROR "Clang 6.0 or newer is required!")
    ENDIF()
  ELSEIF(MY_COMPILER_IS_GNU)
    EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE GCC_VERSION)
    IF(GCC_VERSION VERSION_LESS 5.3)
      MESSAGE(FATAL_ERROR
        "GCC 5.3 or newer is required (-dumpversion says ${GCC_VERSION})")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "Unsupported compiler!")
  ENDIF()
ENDIF()

