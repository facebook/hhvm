/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/hash/Hash.h>
#include <quic/codec/QuicConnectionId.h>

#include <cstdint>
#include <limits>

namespace proxygen {

inline bool shouldLogQuicConnection(const quic::ConnectionId& connId,
                                    int64_t salt,
                                    uint32_t weight) {
  // Logging is off
  if (weight == 0) {
    return false;
  }
  // Hash the combination of connection id and salt.
  // If the hash value is less than or equal to (2^32-1)/weight, log,
  // in which case sample ratio is 1/weight.

  // Hash code stolen from folly::hash::hash_combine_generic.
  // Reason why not using hash_combine_generic directly is that we want to
  // generate a hash value whose type size is consistent across systems,
  // so that server and client can be sure that they are logging the same
  // connection. However hash_combine_generic returns a size_t type which does
  // not satisfy this requirement.
  uint32_t connIdNumHash = folly::hash::fnv32_buf(connId.data(), connId.size());
  uint32_t saltHash = folly::hash::fnv32_buf(&salt, sizeof(salt));
  uint32_t hash =
      folly::hash::twang_32from64((uint64_t(connIdNumHash) << 32) | saltHash);
  if (hash <= std::numeric_limits<uint32_t>::max() / weight) {
    return true;
  }
  return false;
}

} // namespace proxygen
