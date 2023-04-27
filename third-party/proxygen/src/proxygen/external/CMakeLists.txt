# Copyright (c) Facebook, Inc. and its affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

add_library(
    proxygen_http_parser OBJECT
    http_parser/http_parser_cpp.cpp
)

target_compile_options(
    proxygen_http_parser PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
    "-DHTTP_PARSER_STRICT_URL=1"
    "-Wno-implicit-fallthrough"
)

if (BUILD_SHARED_LIBS)
  set_property(TARGET proxygen_http_parser PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(
    proxygen_http_parser PRIVATE
    ${PROXYGEN_FBCODE_ROOT}
)
install(FILES http_parser/http_parser.h DESTINATION
        include/proxygen/external/http_parser)
