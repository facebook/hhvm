# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Manually maintained (generate_cmake.py skips this directory)

proxygen_add_library(libhttperf2
  EXCLUDE_FROM_MONOLITH
  SRCS
    HTTPerf2.cpp
    Client.cpp
  EXPORTED_DEPS
    proxygenhqloggerhelper
    proxygen
)

add_executable(proxygen_httperf2 Main.cpp)
target_link_libraries(
    proxygen_httperf2 PUBLIC
    libhttperf2
)
target_compile_options(
    proxygen_httperf2 PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)

install(
    TARGETS proxygen_httperf2
    EXPORT proxygen-exports
    DESTINATION bin
)
