# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

add_executable(proxygen_h3datagram_client H3DatagramClient.cpp)
target_compile_options(
    proxygen_h3datagram_client PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)
target_link_libraries(
    proxygen_h3datagram_client
    PUBLIC
        proxygen
)
install(
    TARGETS proxygen_h3datagram_client
    EXPORT proxygen-exports
    DESTINATION bin
)
