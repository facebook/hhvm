/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/Executor.h>
#include <folly/Memory.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <wangle/client/persistence/PersistentCache.h>
#include <wangle/client/persistence/PersistentCacheCommon.h>
#include <wangle/client/ssl/SSLSessionCacheData.h>
#include <wangle/client/ssl/SSLSessionCallbacks.h>

#include <atomic>
#include <chrono>
#include <map>

#include <openssl/ssl.h>

namespace wangle {

/**
 * This cache is as threadsafe as the underlying PersistentCache used.
 * Multiple instances may delegate to the same persistence layer
 */
template <typename K>
class SSLSessionPersistentCacheBase : public SSLSessionCallbacks {
 public:
  class TimeUtil {
   public:
    virtual ~TimeUtil() = default;

    virtual std::chrono::time_point<std::chrono::system_clock> now() const {
      return std::chrono::system_clock::now();
    }
  };

  explicit SSLSessionPersistentCacheBase(
      std::shared_ptr<PersistentCache<K, SSLSessionCacheData>> cache);

  SSLSessionPersistentCacheBase(
      const std::string& filename,
      PersistentCacheConfig config);

  // Store the session data of the specified identity in cache. Note that the
  // implementation must make it's own memory copy of the session data to put
  // into the cache.
  void setSSLSession(
      const std::string& identity,
      folly::ssl::SSLSessionUniquePtr session) noexcept override;

  // Return a SSL session if the cache contained session information for the
  // specified identity. It is the caller's responsibility to decrement the
  // reference count of the returned session pointer.
  folly::ssl::SSLSessionUniquePtr getSSLSession(
      const std::string& identity) const noexcept override;

  // Remove session data of the specified identity from cache. Return true if
  // there was session data associated with the identity before removal, or
  // false otherwise.
  bool removeSSLSession(const std::string& identity) noexcept override;

  // Return true if the underlying cache supports persistence
  bool supportsPersistence() const noexcept override {
    return true;
  }

  void setTimeUtil(std::unique_ptr<TimeUtil> timeUtil) noexcept {
    timeUtil_ = std::move(timeUtil);
  }

  // For test only, returns the number of entries in the cache.
  size_t size() const override;

 protected:
  // Get the persistence key from the session's identity
  virtual K getKey(const std::string& identity) const = 0;

  std::shared_ptr<PersistentCache<K, SSLSessionCacheData>> persistentCache_;
  std::unique_ptr<TimeUtil> timeUtil_;
};

class SSLSessionPersistentCache
    : public SSLSessionPersistentCacheBase<std::string> {
 public:
  SSLSessionPersistentCache(
      const std::string& filename,
      PersistentCacheConfig config)
      : SSLSessionPersistentCacheBase(filename, std::move(config)) {}

 protected:
  std::string getKey(const std::string& identity) const override {
    return identity;
  }
};
} // namespace wangle

#include <wangle/client/ssl/SSLSessionPersistentCache-inl.h>
