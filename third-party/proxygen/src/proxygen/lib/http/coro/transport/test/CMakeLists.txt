# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

if(NOT BUILD_TESTS)
    return()
endif()

# Test-only fake coro transport used by qmux and coro transport tests.
# Linked against granular proxygen targets (not the monolithic `proxygen`).
add_library(TestCoroTransport TestCoroTransport.cpp)
target_include_directories(
    TestCoroTransport PUBLIC
    $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
    $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
    ${LIBGMOCK_INCLUDE_DIR}
    ${LIBGTEST_INCLUDE_DIR}
)
target_compile_options(
    TestCoroTransport PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)
target_compile_features(TestCoroTransport PUBLIC cxx_std_20)
target_link_libraries(TestCoroTransport PUBLIC
    proxygen_http_coro_util_cancellable_baton
    Folly::folly
    Folly::folly_io_coro_socket
    ${LIBGMOCK_LIBRARIES}
)
