/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <folly/Optional.h>
#include <folly/io/IOBuf.h>
#include <string>

namespace proxygen {

// Result of expanding a CONNECT-UDP URI template.
struct ConnectUDPTarget {
  std::string scheme;    // e.g., "https"
  std::string authority; // e.g., "127.0.0.1:4443"
  std::string path;      // e.g., "/masque?h=192.0.2.6&p=443"
};

// Expand a CONNECT-UDP URI template (RFC 6570 Level 3).
// Replaces {target_host} and {target_port}. Percent-encodes IPv6 colons
// as %3A in the host value.
// Example: expandConnectUDPTemplate(
//   "https://proxy:4443/masque?h={target_host}&p={target_port}",
//   "192.0.2.6", 443)
// returns {scheme="https", authority="proxy:4443",
//          path="/masque?h=192.0.2.6&p=443"}
ConnectUDPTarget expandConnectUDPTemplate(const std::string& uriTemplate,
                                          const std::string& targetHost,
                                          uint16_t targetPort);

// Prepend QUIC varint context ID 0 (single byte 0x00) to a datagram payload.
std::unique_ptr<folly::IOBuf> prependContextId(
    std::unique_ptr<folly::IOBuf> payload);

// Strip QUIC varint context ID from datagram.
// Returns the payload if context ID is 0, or folly::none if context ID != 0
// or the datagram is too short to parse.
folly::Optional<std::unique_ptr<folly::IOBuf>> stripContextId(
    std::unique_ptr<folly::IOBuf> datagram);

} // namespace proxygen
