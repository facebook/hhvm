/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/PskCache.h>
#include <folly/Synchronized.h>
#include <folly/container/EvictingCacheMap.h>

namespace fizz {
namespace client {

/**
 * PSK cache that provides synchronization and caps the number of PSKs stored
 * internally. When the limit is reached, the least recently used PSK is
 * evicted.
 */
class SynchronizedLruPskCache : public PskCache {
 public:
  using EvictingPskMap = folly::EvictingCacheMap<std::string, CachedPsk>;
  ~SynchronizedLruPskCache() override = default;
  explicit SynchronizedLruPskCache(uint64_t mapMax);

  folly::Optional<CachedPsk> getPsk(const std::string& identity) override;

  void putPsk(const std::string& identity, CachedPsk psk) override;

  void removePsk(const std::string& identity) override;

 private:
  folly::Synchronized<EvictingPskMap> cache_;
};

} // namespace client
} // namespace fizz
