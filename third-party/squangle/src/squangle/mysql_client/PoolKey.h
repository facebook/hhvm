/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

class PoolKey {
 public:
  // Hashes Connections and Operations waiting for connections based on basic
  // Connection info (ConnectionKey) and Connection Attributes.
  PoolKey(ConnectionKey conn_key, ConnectionOptions conn_opts)
      : connKey(std::move(conn_key)), connOptions(std::move(conn_opts)) {
    options_hash_ = folly::hash::hash_range(
        connOptions.getAttributes().begin(), connOptions.getAttributes().end());
    partial_hash_ =
        folly::hash::hash_combine(connKey.partial_hash(), options_hash_);
    full_hash_ = folly::hash::hash_combine(connKey.hash(), options_hash_);
  }

  bool operator==(const PoolKey& rhs) const {
    return full_hash_ == rhs.full_hash_ && options_hash_ == rhs.options_hash_ &&
        connKey == rhs.connKey;
  }

  bool operator!=(const PoolKey& rhs) const {
    return !(*this == rhs);
  }

  bool partialCompare(const PoolKey& rhs) const {
    return partial_hash_ == rhs.partial_hash_ &&
        options_hash_ == rhs.options_hash_ && connKey.partialEqual(rhs.connKey);
  }

  const ConnectionKey* getConnectionKey() const {
    return &connKey;
  }

  const ConnectionKey connKey;
  const ConnectionOptions connOptions;

  size_t getHash() const {
    return full_hash_;
  }

  size_t getPartialHash() const {
    return partial_hash_;
  }

  size_t getOptionsHash() const {
    return options_hash_;
  }

 private:
  size_t options_hash_;
  size_t full_hash_;
  size_t partial_hash_;
};

struct PoolKeyStats {
  size_t open_connections;
  size_t pending_connections;
  size_t connection_limit;
};

std::ostream& operator<<(std::ostream& os, const PoolKey& key);

class PoolKeyHash {
 public:
  size_t operator()(const PoolKey& k) const {
    return k.getHash();
  }
};

class PoolKeyPartialHash {
 public:
  size_t operator()(const PoolKey& k) const {
    return k.getPartialHash();
  }

  bool operator()(const PoolKey& lhs, const PoolKey& rhs) const {
    return lhs.partialCompare(rhs);
  }
};

} // namespace facebook::common::mysql_client
