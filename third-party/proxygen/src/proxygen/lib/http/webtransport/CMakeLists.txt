# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

add_library(
    quicwebtransport
    QuicWebTransport.cpp
)
add_dependencies(
    quicwebtransport
    proxygen
)
target_include_directories(
    quicwebtransport PUBLIC
    $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
    $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
    $<INSTALL_INTERFACE:include/>
    ${LIBGMOCK_INCLUDE_DIR}
    ${LIBGTEST_INCLUDE_DIR}
)
target_compile_options(
    quicwebtransport PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)
target_link_libraries(quicwebtransport PUBLIC proxygen)
install(
    TARGETS quicwebtransport
    EXPORT proxygen-exports
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
)
