# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# - Find zstd
# Find the zstd compression library and includes
#
# ZSTD_INCLUDE_DIR - where to find zstd.h, etc.
# ZSTD_LIBRARIES - List of libraries when using zstd.
# ZSTD_FOUND - True if zstd found.

find_path(ZSTD_INCLUDE_DIR
  NAMES zstd.h
  HINTS ${ZSTD_ROOT_DIR}/include)

find_library(ZSTD_LIBRARIES
  NAMES zstd
  HINTS ${ZSTD_ROOT_DIR}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(zstd DEFAULT_MSG ZSTD_LIBRARIES ZSTD_INCLUDE_DIR)

mark_as_advanced(
  ZSTD_LIBRARIES
  ZSTD_INCLUDE_DIR
)

if(NOT TARGET zstd)
    if("${ZSTD_LIBRARIES}" MATCHES ".*.a$")
        add_library(zstd STATIC IMPORTED)
    else()
        add_library(zstd SHARED IMPORTED)
    endif()
    set_target_properties(
        zstd
        PROPERTIES
            IMPORTED_LOCATION ${ZSTD_LIBRARIES}
            INTERFACE_INCLUDE_DIRECTORIES ${ZSTD_INCLUDE_DIR}
    )
endif()
