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

#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/SSLContext.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <wangle/client/ssl/SSLSessionCacheUtils.h>
#include <wangle/ssl/SSLUtil.h>

#include <openssl/ssl.h>

#ifdef OPENSSL_NO_TLSEXT
#define OPENSSL_TICKETS 0
#else
#define OPENSSL_TICKETS OPENSSL_VERSION_NUMBER >= 0x1000105fL
#endif

namespace wangle {

/**
 * Callbacks related to SSL session cache
 *
 * This class contains three methods, setSSLSession() to store existing SSL
 * session data to cache, getSSLSession() to retreive cached session
 * data in cache, and removeSSLSession() to remove session data from cache.
 */
class SSLSessionCallbacks {
 public:
  /**
   * Store the session data of the specified identity in cache. Note that the
   * implementation must make it's own memory copy of the session data to put
   * into the cache.
   */
  virtual void setSSLSession(
      const std::string& identity,
      folly::ssl::SSLSessionUniquePtr session) noexcept = 0;

  /**
   * Return a SSL session if the cache contained session information for the
   * specified identity. It is the caller's responsibility to decrement the
   * reference count of the returned session pointer.
   */
  virtual folly::ssl::SSLSessionUniquePtr getSSLSession(
      const std::string& identity) const noexcept = 0;

  /**
   * Remove session data of the specified identity from cache. Return true if
   * there was session data associated with the identity before removal, or
   * false otherwise.
   */
  virtual bool removeSSLSession(const std::string& identity) noexcept = 0;

  /**
   * Return true if the underlying cache supports persistence
   */
  virtual bool supportsPersistence() const noexcept {
    return false;
  }

  virtual size_t size() const {
    return 0;
  }

  virtual ~SSLSessionCallbacks() = default;

  /**
   * Sets up SSL Session callbacks on a context.  The application is
   * responsible for detaching the callbacks from the context.
   */
  static void attachCallbacksToContext(
      folly::SSLContext* context,
      SSLSessionCallbacks* callbacks);

  /**
   * Detach the passed in callbacks from the context.  If the callbacks are not
   * set on the context, it is unchanged.
   */
  static void detachCallbacksFromContext(
      folly::SSLContext* context,
      SSLSessionCallbacks* callbacks);

  static SSLSessionCallbacks* getCacheFromContext(SSL_CTX* ctx);

 protected:
  /**
   * Called by ContextSessionCallbacks::onNewSession prior to insertion
   * into the session cache.
   */
  virtual void onNewSession(SSL*, SSL_SESSION*) {}

 private:
  struct ContextSessionCallbacks
      : public folly::SSLContext::SessionLifecycleCallbacks {
    void onNewSession(SSL* ssl, folly::ssl::SSLSessionUniquePtr sessionPtr)
        override;
  };

  static std::string getSessionKeyFromSSL(SSL* ssl);

  static int32_t& getCacheIndex() {
    static int32_t sExDataIndex = -1;
    return sExDataIndex;
  }
};

} // namespace wangle
