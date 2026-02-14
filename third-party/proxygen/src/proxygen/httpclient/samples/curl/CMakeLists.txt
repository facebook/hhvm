# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Manually maintained (generate_cmake.py skips this directory)

proxygen_add_library(proxygencurl
  EXCLUDE_FROM_MONOLITH
  SRCS
    CurlClient.cpp
  EXPORTED_DEPS
    proxygen
)

add_executable(proxygen_curl CurlClientMain.cpp)
target_compile_options(
    proxygen_curl PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)
target_link_libraries(
    proxygen_curl
    PUBLIC
        proxygencurl
)
install(
    TARGETS proxygen_curl
    EXPORT proxygen-exports
    DESTINATION bin
)
