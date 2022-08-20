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

#include <wangle/ssl/SSLSessionCacheManager.h>

#include <wangle/ssl/SSLCacheProvider.h>
#include <wangle/ssl/SSLStats.h>
#include <wangle/ssl/SSLUtil.h>

#include <folly/fibers/Fiber.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/OpenSSL.h>

using folly::AsyncSSLSocket;
using folly::SSLContext;
using std::shared_ptr;
using std::string;

namespace {

const uint32_t NUM_CACHE_BUCKETS = 16;

// We use the default ID generator which fills the maximum ID length
// for the protocol.  16 bytes for SSLv2 or 32 for SSLv3+
const int MIN_SESSION_ID_LENGTH = 16;

} // namespace

DEFINE_bool(dcache_unit_test, false, "All VIPs share one session cache");

namespace wangle {

int SSLSessionCacheManager::sExDataIndex_ = -1;
shared_ptr<ShardedLocalSSLSessionCache> SSLSessionCacheManager::sCache_;
std::mutex SSLSessionCacheManager::sCacheLock_;

LocalSSLSessionCache::LocalSSLSessionCache(
    uint32_t maxCacheSize,
    uint32_t cacheCullSize)
    : sessionCache(maxCacheSize, cacheCullSize) {
  sessionCache.setPruneHook(std::bind(
      &LocalSSLSessionCache::pruneSessionCallback,
      this,
      std::placeholders::_1,
      std::placeholders::_2));
}

void LocalSSLSessionCache::pruneSessionCallback(
    const string& sessionId,
    SSL_SESSION* session) {
  VLOG(4) << "Free SSL session from local cache; id="
          << SSLUtil::hexlify(sessionId);
  SSL_SESSION_free(session);
  ++removedSessions_;
}

// ShardedLocalSSLSessionCache Implementation
ShardedLocalSSLSessionCache::ShardedLocalSSLSessionCache(
    uint32_t n_buckets,
    uint32_t maxCacheSize,
    uint32_t cacheCullSize) {
  CHECK(n_buckets > 0);
  maxCacheSize = (uint32_t)(((double)maxCacheSize) / n_buckets);
  cacheCullSize = (uint32_t)(((double)cacheCullSize) / n_buckets);
  if (maxCacheSize == 0) {
    maxCacheSize = 1;
  }
  if (cacheCullSize == 0) {
    cacheCullSize = 1;
  }
  for (uint32_t i = 0; i < n_buckets; i++) {
    caches_.push_back(std::unique_ptr<LocalSSLSessionCache>(
        new LocalSSLSessionCache(maxCacheSize, cacheCullSize)));
  }
}

SSL_SESSION* ShardedLocalSSLSessionCache::lookupSession(
    const std::string& sessionId) {
  size_t bucket = hash(sessionId);
  SSL_SESSION* session = nullptr;
  std::lock_guard<std::mutex> g(caches_[bucket]->lock);

  auto itr = caches_[bucket]->sessionCache.find(sessionId);
  if (itr != caches_[bucket]->sessionCache.end()) {
    session = itr->second;
  }

  if (session) {
    SSL_SESSION_up_ref(session);
  }
  return session;
}

void ShardedLocalSSLSessionCache::storeSession(
    const std::string& sessionId,
    SSL_SESSION* session,
    SSLStats* stats) {
  size_t bucket = hash(sessionId);
  SSL_SESSION* oldSession = nullptr;
  std::lock_guard<std::mutex> g(caches_[bucket]->lock);

  auto itr = caches_[bucket]->sessionCache.find(sessionId);
  if (itr != caches_[bucket]->sessionCache.end()) {
    oldSession = itr->second;
  }

  if (oldSession) {
    // LRUCacheMap doesn't free on overwrite, so 2x the work for us
    // This can happen in race conditions
    SSL_SESSION_free(oldSession);
  }
  caches_[bucket]->removedSessions_ = 0;
  caches_[bucket]->sessionCache.set(sessionId, session, true);
  if (stats) {
    stats->recordSSLSessionFree(caches_[bucket]->removedSessions_);
  }
}

void ShardedLocalSSLSessionCache::removeSession(const std::string& sessionId) {
  size_t bucket = hash(sessionId);
  std::lock_guard<std::mutex> g(caches_[bucket]->lock);

  auto itr = caches_[bucket]->sessionCache.find(sessionId);
  if (itr == caches_[bucket]->sessionCache.end()) {
    VLOG(4) << "session ID " << sessionId << " not in cache";
    return;
  }

  // LRUCacheMap doesn't free on erase either
  SSL_SESSION_free(itr->second);
  caches_[bucket]->sessionCache.erase(sessionId);
}

// SSLSessionCacheManager implementation

SSLSessionCacheManager::SSLSessionCacheManager(
    uint32_t maxCacheSize,
    uint32_t cacheCullSize,
    SSLContext* ctx,
    const string& context,
    SSLStats* stats,
    const std::shared_ptr<SSLCacheProvider>& externalCache)
    : ctx_(ctx), stats_(stats), externalCache_(externalCache) {
  SSL_CTX* sslCtx = ctx->getSSLCtx();

  SSLUtil::getSSLCtxExIndex(&sExDataIndex_);

  SSL_CTX_set_ex_data(sslCtx, sExDataIndex_, this);
  SSL_CTX_sess_set_get_cb(sslCtx, SSLSessionCacheManager::getSessionCallback);
  SSL_CTX_sess_set_remove_cb(
      sslCtx, SSLSessionCacheManager::removeSessionCallback);
  ctx->setSessionLifecycleCallbacks(
      std::make_unique<ContextSessionCallbacks>());
  if (!FLAGS_dcache_unit_test && !context.empty()) {
    // Use the passed in context
    ctx->setSessionCacheContext(context);
  }

  SSL_CTX_set_session_cache_mode(
      sslCtx, SSL_SESS_CACHE_NO_INTERNAL | SSL_SESS_CACHE_SERVER);

  localCache_ =
      SSLSessionCacheManager::getLocalCache(maxCacheSize, cacheCullSize);
}

SSLSessionCacheManager::~SSLSessionCacheManager() {}

void SSLSessionCacheManager::shutdown() {
  std::lock_guard<std::mutex> g(sCacheLock_);
  sCache_.reset();
}

shared_ptr<ShardedLocalSSLSessionCache> SSLSessionCacheManager::getLocalCache(
    uint32_t maxCacheSize,
    uint32_t cacheCullSize) {
  std::lock_guard<std::mutex> g(sCacheLock_);
  if (!sCache_) {
    sCache_.reset(new ShardedLocalSSLSessionCache(
        NUM_CACHE_BUCKETS, maxCacheSize, cacheCullSize));
  }
  return sCache_;
}

void SSLSessionCacheManager::newSession(SSL*, SSL_SESSION* session) {
  unsigned int sessIdLen = 0;
  const unsigned char* sessId = SSL_SESSION_get_id(session, &sessIdLen);
  string sessionId((char*)sessId, sessIdLen);
  VLOG(4) << "New SSL session; id=" << SSLUtil::hexlify(sessionId);

  if (stats_) {
    stats_->recordSSLSession(true /* new session */, false, false);
  }

  localCache_->storeSession(sessionId, session, stats_);

  if (externalCache_) {
    VLOG(4) << "New SSL session: send session to external cache; id="
            << SSLUtil::hexlify(sessionId);
    storeCacheRecord(sessionId, session);
  }
}

void SSLSessionCacheManager::removeSessionCallback(
    SSL_CTX* ctx,
    SSL_SESSION* session) {
  SSLSessionCacheManager* manager = nullptr;
  manager = (SSLSessionCacheManager*)SSL_CTX_get_ex_data(ctx, sExDataIndex_);

  if (manager == nullptr) {
    LOG(FATAL) << "Null SSLSessionCacheManager in callback";
  }
  return manager->removeSession(ctx, session);
}

void SSLSessionCacheManager::removeSession(SSL_CTX*, SSL_SESSION* session) {
  unsigned int sessIdLen = 0;
  const unsigned char* sessId = SSL_SESSION_get_id(session, &sessIdLen);
  string sessionId((char*)sessId, sessIdLen);

  // This hook is only called from SSL when the internal session cache needs to
  // flush sessions.  Since we run with the internal cache disabled, this should
  // never be called
  VLOG(3) << "Remove SSL session; id=" << SSLUtil::hexlify(sessionId);

  localCache_->removeSession(sessionId);

  if (stats_) {
    stats_->recordSSLSessionRemove();
  }
}

SSL_SESSION* SSLSessionCacheManager::getSessionCallback(
    SSL* ssl,
    session_callback_arg_session_id_t sess_id,
    int id_len,
    int* copyflag) {
  SSLSessionCacheManager* manager = nullptr;
  SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
  manager = (SSLSessionCacheManager*)SSL_CTX_get_ex_data(ctx, sExDataIndex_);

  if (manager == nullptr) {
    LOG(FATAL) << "Null SSLSessionCacheManager in callback";
  }
  return manager->getSession(ssl, (unsigned char*)sess_id, id_len, copyflag);
}

SSL_SESSION* SSLSessionCacheManager::getSession(
    SSL* ssl,
    unsigned char* session_id,
    int id_len,
    int* copyflag) {
  VLOG(7) << "SSL get session callback";
  folly::ssl::SSLSessionUniquePtr session;
  bool foreign = false;
  std::string missReason;

  if (id_len < MIN_SESSION_ID_LENGTH) {
    // We didn't generate this session so it's going to be a miss.
    // This doesn't get logged or counted in the stats.
    return nullptr;
  }
  string sessionId((char*)session_id, id_len);

  AsyncSSLSocket* sslSocket = AsyncSSLSocket::getFromSSL(ssl);

  assert(sslSocket != nullptr);

  // look it up in the local cache first
  session.reset(localCache_->lookupSession(sessionId));
#if FOLLY_OPENSSL_IS_110
  if (session == nullptr && externalCache_) {
    foreign = true;
    DCHECK(folly::fibers::onFiber());
    if (folly::fibers::onFiber()) {
      try {
        session = externalCache_->getFuture(sessionId).get();
      } catch (const std::exception& e) {
        missReason = folly::to<std::string>("reason: ", e.what(), ";");
      }

      if (session) {
        SSL_SESSION_up_ref(session.get());
        localCache_->storeSession(sessionId, session.get(), stats_);
      }
    } else {
      missReason = "reason: request not running on fiber;";
    }
  }
#endif

  bool hit = (session != nullptr);
  if (stats_) {
    stats_->recordSSLSession(false, hit, foreign);
  }
  if (hit) {
    sslSocket->setSessionIDResumed(true);
  }

  VLOG(4) << "Get SSL session [" << ((hit) ? "Hit" : "Miss")
          << "]: " << ((foreign) ? "external" : "local") << " cache; "
          << missReason << "fd=" << sslSocket->getNetworkSocket().toFd()
          << " id=" << SSLUtil::hexlify(sessionId);

  // We already bumped the refcount
  *copyflag = 0;

  return session.release();
}

bool SSLSessionCacheManager::storeCacheRecord(
    const string& sessionId,
    SSL_SESSION* session) {
  std::string sessionString;
  uint32_t sessionLen = i2d_SSL_SESSION(session, nullptr);
  sessionString.resize(sessionLen);
  uint8_t* cp = (uint8_t*)sessionString.data();
  i2d_SSL_SESSION(session, &cp);
  size_t expiration = SSL_CTX_get_timeout(ctx_->getSSLCtx());
  return externalCache_->setAsync(
      sessionId, sessionString, std::chrono::seconds(expiration));
}

void SSLSessionCacheManager::ContextSessionCallbacks::onNewSession(
    SSL* ssl,
    folly::ssl::SSLSessionUniquePtr sessionPtr) {
  SSLSessionCacheManager* manager = nullptr;
  SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
  manager = (SSLSessionCacheManager*)SSL_CTX_get_ex_data(ctx, sExDataIndex_);

  CHECK(manager) << "Null SSLSessionCacheManager in callback";
  manager->newSession(ssl, sessionPtr.release());
}

} // namespace wangle
