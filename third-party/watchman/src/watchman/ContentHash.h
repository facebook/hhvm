/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <array>
#include "watchman/LRUCache.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_system.h"

namespace watchman {
struct ContentHashCacheKey {
  // Path relative to the watched root
  w_string relativePath;
  // file size in bytes
  size_t fileSize;
  // The modification time
  struct timespec mtime;

  // Computes a hash value for use in the cache map
  std::size_t hashValue() const;
  bool operator==(const ContentHashCacheKey& other) const;
};
} // namespace watchman

namespace std {
template <>
struct hash<watchman::ContentHashCacheKey> {
  std::size_t operator()(watchman::ContentHashCacheKey const& key) const {
    return key.hashValue();
  }
};
} // namespace std

namespace watchman {
class ContentHashCache {
 public:
  using HashValue = std::array<uint8_t, 20>;
  using Node = LRUCache<ContentHashCacheKey, HashValue>::NodeType;

  // Construct a cache for a given root, holding the specified
  // maximum number of items, using the configured negative
  // caching TTL.
  ContentHashCache(
      const w_string& rootPath,
      size_t maxItems,
      std::chrono::milliseconds errorTTL);

  // Obtain the content hash for the given input.
  // If the result is in the cache it will return a ready future
  // holding the result.  Otherwise, computeHash will be invoked
  // to populate the cache.  Returns a future with the result
  // of the lookup.
  folly::Future<std::shared_ptr<const Node>> get(
      const ContentHashCacheKey& key);

  // Compute the hash value for a given input.
  // This will block the calling thread while the I/O is performed.
  // Throws exceptions for any errors that may occur.
  HashValue computeHashImmediate(const ContentHashCacheKey& key) const;

  // Compute the hash value for a given input.
  // This will block the calling thread while the I/O is performed.
  // Throws exceptions for any errors that may occur.
  static HashValue computeHashImmediate(const char* fullPath);

  // Compute the hash value for a given input via the thread pool.
  // Returns a future to operate on the result of this async operation
  folly::Future<HashValue> computeHash(const ContentHashCacheKey& key) const;

  // Returns the root path that this cache is associated with
  const w_string& rootPath() const;

  // Returns cache statistics
  CacheStats stats() const;

 private:
  LRUCache<ContentHashCacheKey, HashValue> cache_;
  w_string rootPath_;
};
} // namespace watchman
