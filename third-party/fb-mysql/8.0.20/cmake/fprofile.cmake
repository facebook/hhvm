# Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
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

# Options for doing PGO (profile guided optimization) with gcc/clang.
#
# gcc8 and gcc9 handle naming and location of the .gcda files
# (containing profile data) completely differently, hence the slightly
# different HowTos below:
#
# HowTo for gcc8:
# Assuming we have three build directories
#   <some path>/build-gen
#   <some path>/build-use
#   <some path>/profile-data
#
# in build-gen
#   cmake <path to source> -DFPROFILE_GENERATE=1
#   make
#   rm -rf ../profile-data
#   run whatever test suite is an appropriate training set
# in build-use
#   cmake <path to source> -DFPROFILE_USE=1
#   make
#
# HowTo for gcc9:
# Assuming we have two build directories
#   <some path>/build
#   <some path>/profile-data
#
# in build
#   cmake <path to source> -DFPROFILE_GENERATE=1
#   make
#   rm -rf ../profile-data
#   run whatever test suite is an appropriate training set
# now rename the build directory to something else, or simply 'rm -rf' it.
# 'mkdir build' note: same name
# in the new build
#   cmake <path to source> -DFPROFILE_USE=1
#   make
#
# HowTo for clang:
# Assuming we have three build directories
#   <some path>/build-gen
#   <some path>/build-use
#   <some path>/profile-data
#
# in build-gen
#   cmake <path to source> -DFPROFILE_GENERATE=1
#   make
#   rm -rf ../profile-data
#   run whatever test suite is an appropriate training set
# in profile-data
#   llvm-profdata merge -output=default.profdata .
# in build-use
#   cmake <path to source> -DFPROFILE_USE=1
#   make
#
#
# Your executables should hopefully be faster than a default build.
# In order to share the profile data, we turn on REPRODUCIBLE_BUILD.
# We also switch off USE_LD_LLD since it resulted in some linking problems.

# Currently only implemented for gcc and clang.
IF(NOT MY_COMPILER_IS_GNU_OR_CLANG)
  RETURN()
ENDIF()

SET(FPROFILE_DIR_DEFAULT "${CMAKE_BINARY_DIR}/../profile-data")
IF(NOT DEFINED FPROFILE_DIR)
  SET(FPROFILE_DIR "${FPROFILE_DIR_DEFAULT}")
ENDIF()

OPTION(FPROFILE_GENERATE "Add -fprofile-generate" OFF)
IF(FPROFILE_GENERATE)
  STRING_APPEND(CMAKE_C_FLAGS " -fprofile-generate=${FPROFILE_DIR}")
  STRING_APPEND(CMAKE_CXX_FLAGS " -fprofile-generate=${FPROFILE_DIR}")
ENDIF()

OPTION(FPROFILE_USE "Add -fprofile-use" OFF)
IF(FPROFILE_USE)
  STRING_APPEND(CMAKE_C_FLAGS " -fprofile-use=${FPROFILE_DIR}")
  STRING_APPEND(CMAKE_CXX_FLAGS " -fprofile-use=${FPROFILE_DIR}")
  # Collection of profile data is not thread safe, use -fprofile-correction for GCC
  IF(MY_COMPILER_IS_GNU)
    STRING_APPEND(CMAKE_C_FLAGS " -fprofile-correction")
    STRING_APPEND(CMAKE_CXX_FLAGS " -fprofile-correction")
  ENDIF()
ENDIF()

IF(FPROFILE_GENERATE AND FPROFILE_USE)
  MESSAGE(FATAL_ERROR "Cannot combine -fprofile-generate and -fprofile-use")
ENDIF()

IF(FPROFILE_GENERATE OR FPROFILE_USE)
  SET(REPRODUCIBLE_BUILD ON CACHE INTERNAL "")
  # Build fails with lld, so switch it off.
  SET(USE_LD_LLD OFF CACHE INTERNAL "")
ENDIF()

IF(FPROFILE_USE)
  # LTO combined with PGO boosts performance even more.
  SET(WITH_LTO_DEFAULT ON CACHE INTERNAL "")
ENDIF()
