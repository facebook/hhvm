/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/client/PskCache.h>
#include <fizz/client/PskSerializationUtils.h>
#include <wangle/client/persistence/FilePersistentCache.h>

namespace proxygen {

struct PersistentCachedPsk {
  std::string serialized;
  size_t uses{0};
};

class PersistentFizzPskCache : public fizz::client::PskCache {
 public:
  ~PersistentFizzPskCache() override = default;

  PersistentFizzPskCache(const std::string& filename,
                         wangle::PersistentCacheConfig config)
      : cache_(filename, std::move(config)) {
  }

  void setMaxPskUses(size_t maxUses) {
    maxPskUses_ = maxUses;
  }

  /**
   * Returns number of times the psk has been used.
   */
  folly::Optional<size_t> getPskUses(const std::string& identity) {
    auto serialized = cache_.get(identity);
    if (serialized) {
      return serialized->uses;
    }
    return folly::none;
  }

  /**
   * Returns a PSK, without increasing its usage count. Keep in mind that it
   * would still update the internal LRU cache
   */
  folly::Optional<fizz::client::CachedPsk> peekPsk(
      const std::string& identity) {
    auto serialized = cache_.get(identity);
    if (serialized) {
      try {
        return fizz::client::deserializePsk(
            fizz::openssl::certificateSerializer(),
            folly::ByteRange(serialized->serialized));
      } catch (const std::exception& ex) {
        LOG(ERROR) << "Error deserializing PSK: " << ex.what();
        cache_.remove(identity);
      }
    }
    return folly::none;
  }

  folly::Optional<fizz::client::CachedPsk> getPsk(
      const std::string& identity) override {
    auto serialized = cache_.get(identity);
    if (serialized) {
      try {
        auto deserialized = fizz::client::deserializePsk(
            fizz::openssl::certificateSerializer(),
            folly::ByteRange(serialized->serialized));
        serialized->uses++;
        if (maxPskUses_ != 0 && serialized->uses >= maxPskUses_) {
          cache_.remove(identity);
        } else {
          cache_.put(identity, *serialized);
        }
        return deserialized;
      } catch (const std::exception& ex) {
        LOG(ERROR) << "Error deserializing PSK: " << ex.what();
        cache_.remove(identity);
      }
    }
    return folly::none;
  }

  void putPsk(const std::string& identity,
              fizz::client::CachedPsk psk) override {
    PersistentCachedPsk serialized;
    serialized.serialized =
        fizz::client::serializePsk(fizz::openssl::certificateSerializer(), psk);
    serialized.uses = 0;
    cache_.put(identity, serialized);
  }

  void removePsk(const std::string& identity) override {
    cache_.remove(identity);
  }

 private:
  wangle::FilePersistentCache<std::string, PersistentCachedPsk> cache_;

  size_t maxPskUses_{5};
};
} // namespace proxygen

namespace folly {

template <>
dynamic toDynamic(const proxygen::PersistentCachedPsk& cached);
template <>
proxygen::PersistentCachedPsk convertTo(const dynamic& d);
} // namespace folly
