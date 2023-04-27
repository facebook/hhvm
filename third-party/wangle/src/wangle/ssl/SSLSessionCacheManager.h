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

#include <wangle/ssl/SSLCacheProvider.h>
#include <wangle/ssl/SSLStats.h>

#include <folly/container/EvictingCacheMap.h>
#include <folly/hash/Hash.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <mutex>

namespace wangle {

class SSLStats;

/**
 * Basic SSL session cache map: Maps session id -> session
 */
typedef folly::EvictingCacheMap<std::string, SSL_SESSION*> SSLSessionCacheMap;

/**
 * Holds an SSLSessionCacheMap and associated lock
 */
class LocalSSLSessionCache {
 public:
  LocalSSLSessionCache(uint32_t maxCacheSize, uint32_t cacheCullSize);

  ~LocalSSLSessionCache() {
    std::lock_guard<std::mutex> g(lock);
    // EvictingCacheMap dtor doesn't free values
    sessionCache.clear();
  }

  SSLSessionCacheMap sessionCache;
  std::mutex lock;
  uint32_t removedSessions_{0};

 private:
  LocalSSLSessionCache(const LocalSSLSessionCache&) = delete;
  LocalSSLSessionCache& operator=(const LocalSSLSessionCache&) = delete;

  void pruneSessionCallback(const std::string& sessionId, SSL_SESSION* session);
};

/**
 * A sharded LRU for SSL sessions.  The sharding is inteneded to reduce
 * contention for the LRU locks.  Assuming uniform distribution, two workers
 * will contend for the same lock with probability 1 / n_buckets^2.
 */
class ShardedLocalSSLSessionCache {
 public:
  ShardedLocalSSLSessionCache(
      uint32_t n_buckets,
      uint32_t maxCacheSize,
      uint32_t cacheCullSize);

  SSL_SESSION* lookupSession(const std::string& sessionId);

  void storeSession(
      const std::string& sessionId,
      SSL_SESSION* session,
      SSLStats* stats);

  void removeSession(const std::string& sessionId);

  size_t hash(const std::string& key) {
    return folly::Hash()(key) % caches_.size();
  }

  std::vector<std::unique_ptr<LocalSSLSessionCache>> caches_;

 private:
  ShardedLocalSSLSessionCache(const ShardedLocalSSLSessionCache&) = delete;
  ShardedLocalSSLSessionCache& operator=(const ShardedLocalSSLSessionCache&) =
      delete;
};

/**
 * SSLSessionCacheManager handles all stateful session caching.  There is an
 * instance of this object per SSL VIP per thread, with a 1:1 correlation with
 * SSL_CTX.  The cache can work locally or in concert with an external cache
 * to share sessions across instances.
 *
 * There is a single in memory session cache shared by all VIPs.  The cache is
 * split into N buckets (currently 16) with a separate lock per bucket.  The
 * VIP ID is hashed and stored as part of the session to handle the
 * (very unlikely) case of session ID collision.
 *
 * When a new SSL session is created, it is added to the LRU cache and
 * sent to the external cache to be stored.  The external cache
 * expiration is equal to the SSL session's expiration.
 *
 * When a resume request is received, SSLSessionCacheManager first looks in the
 * local LRU cache for the VIP.  If there is a miss there, an asynchronous
 * request for this session is dispatched to the external cache.  When the
 * external cache query returns, the LRU cache is updated if the session was
 * found, and the SSL_accept call is resumed.
 *
 */
class SSLSessionCacheManager {
 public:
  /**
   * Constructor.  SSL session related callbacks will be set on the underlying
   * SSL_CTX.  vipId is assumed to a unique string identifying the VIP and must
   * be the same on all servers that wish to share sessions via the same
   * external cache.
   */
  SSLSessionCacheManager(
      uint32_t maxCacheSize,
      uint32_t cacheCullSize,
      folly::SSLContext* ctx,
      const std::string& context,
      SSLStats* stats,
      const std::shared_ptr<SSLCacheProvider>& externalCache);

  virtual ~SSLSessionCacheManager();

  /**
   * Call this on shutdown to release the global instance of the
   * ShardedLocalSSLSessionCache.
   */
  static void shutdown();

 private:
  SSLSessionCacheManager(const SSLSessionCacheManager&) = delete;
  SSLSessionCacheManager& operator=(const SSLSessionCacheManager&) = delete;

 private:
  struct ContextSessionCallbacks
      : public folly::SSLContext::SessionLifecycleCallbacks {
    void onNewSession(SSL* ssl, folly::ssl::SSLSessionUniquePtr sessionPtr)
        override;
  };

  folly::SSLContext* ctx_;
  std::shared_ptr<ShardedLocalSSLSessionCache> localCache_;
  SSLStats* stats_{nullptr};
  std::shared_ptr<SSLCacheProvider> externalCache_;

  /**
   * Invoked by openssl when a new SSL session is created
   */
  void newSession(SSL* ssl, SSL_SESSION* session);

  /**
   * Invoked by openssl when an SSL session is ejected from its internal cache.
   * This can't be invoked in the current implementation because SSL's internal
   * caching is disabled.
   */
  void removeSession(SSL_CTX* ctx, SSL_SESSION* session);

  /**
   * Invoked by openssl when a client requests a stateful session resumption.
   * Triggers a lookup in our local cache and potentially an asynchronous
   * request to an external cache.
   */
  SSL_SESSION*
  getSession(SSL* ssl, unsigned char* session_id, int id_len, int* copyflag);

  /**
   * Store a new session record in the external cache
   */
  bool storeCacheRecord(const std::string& sessionId, SSL_SESSION* session);

  /**
   * Get or create the LRU cache for the given VIP ID
   */
  static std::shared_ptr<ShardedLocalSSLSessionCache> getLocalCache(
      uint32_t maxCacheSize,
      uint32_t cacheCullSize);

  /**
   * static functions registered as callbacks to openssl via
   * SSL_CTX_sess_set_new/get/remove_cb
   */
  static void removeSessionCallback(SSL_CTX* ctx, SSL_SESSION* session);

#if FOLLY_OPENSSL_IS_110
  using session_callback_arg_session_id_t = const unsigned char*;
#else
  using session_callback_arg_session_id_t = unsigned char*;
#endif

  static SSL_SESSION* getSessionCallback(
      SSL* ssl,
      session_callback_arg_session_id_t session_id,
      int id_len,
      int* copyflag);

  static int32_t sExDataIndex_;
  static std::shared_ptr<ShardedLocalSSLSessionCache> sCache_;
  static std::mutex sCacheLock_;
};

} // namespace wangle
