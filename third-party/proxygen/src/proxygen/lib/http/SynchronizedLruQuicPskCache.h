/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Synchronized.h>
#include <folly/container/EvictingCacheMap.h>
#include <quic/fizz/client/handshake/QuicPskCache.h>

namespace proxygen {

class SynchronizedLruQuicPskCache : public quic::QuicPskCache {
 public:
  ~SynchronizedLruQuicPskCache() override = default;

  explicit SynchronizedLruQuicPskCache(uint64_t mapMax);

  folly::Optional<quic::QuicCachedPsk> getPsk(
      const std::string& identity) override;

  void putPsk(const std::string& identity, quic::QuicCachedPsk psk) override;

  void removePsk(const std::string& identity) override;

 private:
  using EvictingPskMap =
      folly::EvictingCacheMap<std::string, quic::QuicCachedPsk>;
  folly::Synchronized<EvictingPskMap> cache_;
};

} // namespace proxygen
