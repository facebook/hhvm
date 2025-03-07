/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/SymlinkTargets.h"
#include "watchman/Hash.h"
#include "watchman/ThreadPool.h"
#include "watchman/fs/FileSystem.h"

namespace watchman {

using Node = typename SymlinkTargetCache::Node;

bool SymlinkTargetCacheKey::operator==(
    const SymlinkTargetCacheKey& other) const {
  return otime.ticks == other.otime.ticks && relativePath == other.relativePath;
}

std::size_t SymlinkTargetCacheKey::hashValue() const {
  return hash_128_to_64(relativePath.hashValue(), otime.ticks);
}

SymlinkTargetCache::SymlinkTargetCache(
    const w_string& rootPath,
    size_t maxItems,
    std::chrono::milliseconds errorTTL)
    : cache_(maxItems, errorTTL), rootPath_(rootPath) {}

folly::Future<std::shared_ptr<const Node>> SymlinkTargetCache::get(
    const SymlinkTargetCacheKey& key) {
  return cache_.get(
      key, [this](const SymlinkTargetCacheKey& k) { return readLink(k); });
}

w_string SymlinkTargetCache::readLinkImmediate(
    const SymlinkTargetCacheKey& key) const {
  auto fullPath = w_string::pathCat({rootPath_, key.relativePath});
  return readSymbolicLink(fullPath.c_str());
}

folly::Future<w_string> SymlinkTargetCache::readLink(
    const SymlinkTargetCacheKey& key) const {
  return folly::makeFuture(key)
      .via(&getThreadPool())
      .thenValue(
          [this](SymlinkTargetCacheKey key) { return readLinkImmediate(key); });
}

const w_string& SymlinkTargetCache::rootPath() const {
  return rootPath_;
}

CacheStats SymlinkTargetCache::stats() const {
  return cache_.stats();
}
} // namespace watchman
