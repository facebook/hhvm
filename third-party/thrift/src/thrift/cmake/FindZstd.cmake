# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# - Try to find Facebook zstd library
# This will define
# ZSTD_FOUND
# ZSTD_INCLUDE_DIR
# ZSTD_LIBRARIES
#

find_path(
  ZSTD_INCLUDE_DIRS zstd.h
  HINTS
      $ENV{ZSTD_ROOT}/include
      ${ZSTD_ROOT}/include
)

find_library(
    ZSTD_LIBRARIES zstd zstd_static
    HINTS
        $ENV{ZSTD_ROOT}/lib
        ${ZSTD_ROOT}/lib
)

mark_as_advanced(ZSTD_INCLUDE_DIRS ZSTD_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Zstd ZSTD_INCLUDE_DIRS ZSTD_LIBRARIES)

if(ZSTD_FOUND AND NOT ZSTD_FIND_QUIETLY)
    message(STATUS "ZSTD: ${ZSTD_INCLUDE_DIRS}")
endif()
