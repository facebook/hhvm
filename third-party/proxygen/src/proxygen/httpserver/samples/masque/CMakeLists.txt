# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

add_executable(masque_client
  MasqueClient.cpp
)
target_compile_options(
    masque_client PRIVATE ${_PROXYGEN_COMMON_COMPILE_OPTIONS})
target_link_libraries(masque_client
  PUBLIC
    proxygen_transport_h3_datagram_async_socket
    proxygen_transport_connect_udp_utils
    proxygen_hq_insecure_verifier
    proxygen
    Folly::folly
    Folly::folly_init_init
    Folly::folly_portability_gflags
)
install(TARGETS masque_client EXPORT proxygen-exports DESTINATION bin)
