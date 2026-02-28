/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/Types.h>
#include <fizz/record/Types.h>
#include <chrono>
#include <unordered_map>

namespace fizz {
namespace client {

struct CachedPsk {
  std::string psk;
  std::string secret;
  PskType type;

  ProtocolVersion version;
  CipherSuite cipher;
  folly::Optional<NamedGroup> group;
  std::shared_ptr<const Cert> serverCert;
  std::shared_ptr<const Cert> clientCert;

  uint32_t maxEarlyDataSize{0};
  folly::Optional<std::string> alpn;

  uint32_t ticketAgeAdd;
  std::chrono::system_clock::time_point ticketIssueTime;
  std::chrono::system_clock::time_point ticketExpirationTime;
  std::chrono::system_clock::time_point ticketHandshakeTime;
};

class PskCache {
 public:
  virtual ~PskCache() = default;

  /**
   * Retrieve a PSK for the specified identity.
   */
  virtual folly::Optional<CachedPsk> getPsk(const std::string& identity) = 0;

  /**
   * Add a new PSK for identity to the cache.
   */
  virtual void putPsk(const std::string& identity, CachedPsk) = 0;

  /**
   * Remove any PSKs associated with identity from the cache.
   */
  virtual void removePsk(const std::string& identity) = 0;
};

/**
 * Basic PSK cache that stores PSKs in a hash map. There is no bound on the size
 * of this cache.
 */
class BasicPskCache : public PskCache {
 public:
  ~BasicPskCache() override = default;

  folly::Optional<CachedPsk> getPsk(const std::string& identity) override {
    auto result = cache_.find(identity);
    if (result != cache_.end()) {
      return result->second;
    } else {
      return folly::none;
    }
  }

  void putPsk(const std::string& identity, CachedPsk psk) override {
    cache_[identity] = std::move(psk);
  }

  void removePsk(const std::string& identity) override {
    cache_.erase(identity);
  }

 private:
  std::unordered_map<std::string, CachedPsk> cache_;
};
} // namespace client
} // namespace fizz
