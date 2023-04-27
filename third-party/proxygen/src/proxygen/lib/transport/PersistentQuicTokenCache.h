/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <quic/fizz/client/handshake/QuicTokenCache.h>
#include <wangle/client/persistence/FilePersistentCache.h>

namespace proxygen {

class PersistentQuicTokenCache : public quic::QuicTokenCache {
 public:
  PersistentQuicTokenCache(const std::string& filename,
                           wangle::PersistentCacheConfig config);

  [[nodiscard]] folly::Optional<std::string> getToken(
      const std::string& hostname) override;

  void putToken(const std::string& hostname, std::string token) override;

  void removeToken(const std::string&) override;

 private:
  wangle::FilePersistentCache<std::string, std::string> cache_;
};

} // namespace proxygen
