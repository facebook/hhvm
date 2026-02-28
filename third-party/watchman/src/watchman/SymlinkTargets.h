/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <string>
#include "watchman/Clock.h"
#include "watchman/LRUCache.h"
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_system.h"

namespace watchman {
struct SymlinkTargetCacheKey {
  // Path relative to the watched root
  w_string relativePath;
  // The modification time
  ClockStamp otime;

  // Computes a hash value for use in the cache map
  std::size_t hashValue() const;
  bool operator==(const SymlinkTargetCacheKey& other) const;
};
} // namespace watchman

namespace std {
template <>
struct hash<watchman::SymlinkTargetCacheKey> {
  std::size_t operator()(watchman::SymlinkTargetCacheKey const& key) const {
    return key.hashValue();
  }
};
} // namespace std

namespace watchman {
class SymlinkTargetCache {
 public:
  using Node = LRUCache<SymlinkTargetCacheKey, w_string>::NodeType;

  // Construct a cache for a given root, holding the specified
  // maximum number of items, using the configured negative
  // caching TTL.
  SymlinkTargetCache(
      const w_string& rootPath,
      size_t maxItems,
      std::chrono::milliseconds errorTTL);

  // Obtain the content hash for the given input.
  // If the result is in the cache it will return a ready future
  // holding the result.  Otherwise, computeHash will be invoked
  // to populate the cache.  Returns a future with the result
  // of the lookup.
  folly::Future<std::shared_ptr<const Node>> get(
      const SymlinkTargetCacheKey& key);

  // Read the symlink target.
  // This will block the calling thread while the I/O is performed.
  // Throws exceptions for any errors that may occur.
  w_string readLinkImmediate(const SymlinkTargetCacheKey& key) const;

  // Read the symlink target for a given input via the thread pool.
  // Returns a future to operate on the result of this async operation
  folly::Future<w_string> readLink(const SymlinkTargetCacheKey& key) const;

  // Returns the root path that this cache is associated with
  const w_string& rootPath() const;

  // Returns cache statistics
  CacheStats stats() const;

 private:
  LRUCache<SymlinkTargetCacheKey, w_string> cache_;
  w_string rootPath_;
};
} // namespace watchman
