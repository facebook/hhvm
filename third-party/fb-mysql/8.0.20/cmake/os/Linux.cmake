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

# This file includes Linux specific options and quirks, related to system checks

INCLUDE(CheckSymbolExists)
INCLUDE(CheckCSourceRuns)

SET(LINUX 1)

# OS display name (version_compile_os etc).
# Used by the test suite to ignore bugs on some platforms.
SET(SYSTEM_TYPE "Linux")

IF(EXISTS "/etc/SuSE-release")
  SET(LINUX_SUSE 1)
ENDIF()

IF(EXISTS "/etc/alpine-release")
  SET(LINUX_ALPINE 1)
ENDIF()

IF(EXISTS "/etc/fedora-release")
  SET(LINUX_FEDORA 1)
  FILE(READ "/etc/fedora-release" FEDORA_RELEASE)
  IF(FEDORA_RELEASE MATCHES "Fedora" AND
      FEDORA_RELEASE MATCHES "28")
    SET(LINUX_FEDORA_28 1)
  ENDIF()
ENDIF()

IF(EXISTS "/etc/os-release")
  FILE(READ "/etc/os-release" MY_OS_RELEASE)
  IF(MY_OS_RELEASE MATCHES "Ubuntu" AND
      MY_OS_RELEASE MATCHES "16.04")
    SET(LINUX_UBUNTU_16_04 1)
  ENDIF()
  IF(MY_OS_RELEASE MATCHES "Debian")
    SET(LINUX_DEBIAN 1)
  ELSEIF(MY_OS_RELEASE MATCHES "Ubuntu")
    SET(LINUX_UBUNTU 1)
  ENDIF()
ENDIF()

# We require at least GCC 5.3 or Clang 3.4.
IF(NOT FORCE_UNSUPPORTED_COMPILER)
  IF(MY_COMPILER_IS_GNU)
    EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    OUTPUT_VARIABLE GCC_VERSION)
    # -dumpversion may output only MAJOR.MINOR rather than MAJOR.MINOR.PATCH
    IF(GCC_VERSION VERSION_LESS 5.3)
      SET(WARNING_LEVEL WARNING)
      IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.3)
        SET(WARNING_LEVEL FATAL_ERROR)
      ENDIF()
      MESSAGE(${WARNING_LEVEL}
        "GCC 5.3 or newer is required (-dumpversion says ${GCC_VERSION})")
    ENDIF()
  ELSEIF(MY_COMPILER_IS_CLANG)
    CHECK_C_SOURCE_RUNS("
      int main()
      {
        return (__clang_major__ < 3) ||
               (__clang_major__ == 3 && __clang_minor__ < 4);
      }" HAVE_SUPPORTED_CLANG_VERSION)
    IF(NOT HAVE_SUPPORTED_CLANG_VERSION)
      MESSAGE(FATAL_ERROR "Clang 3.4 or newer is required!")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "Unsupported compiler!")
  ENDIF()
ENDIF()

# ISO C89, ISO C99, POSIX.1, POSIX.2, BSD, SVID, X/Open, LFS, and GNU extensions.
ADD_DEFINITIONS(-D_GNU_SOURCE)

# 64 bit file offset support flag
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)

# Ensure we have clean build for shared libraries
# without unresolved symbols
# Not supported with Sanitizers
IF(NOT WITH_ASAN AND
   NOT WITH_LSAN AND
   NOT WITH_MSAN AND
   NOT WITH_TSAN AND
   NOT WITH_UBSAN)
  SET(LINK_FLAG_NO_UNDEFINED "-Wl,--no-undefined")
ENDIF()

# Linux specific HUGETLB /large page support
CHECK_SYMBOL_EXISTS(SHM_HUGETLB sys/shm.h HAVE_LINUX_LARGE_PAGES)
