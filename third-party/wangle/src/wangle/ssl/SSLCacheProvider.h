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

#include <folly/futures/Future.h>
#include <folly/io/async/AsyncSSLSocket.h>

namespace wangle {

class SSLSessionCacheManager;

/**
 * Interface to be implemented by providers of external session caches
 */
class SSLCacheProvider {
 public:
  /**
   * Context saved during an external cache request that is used to
   * resume the waiting client.
   */
  struct CacheContext {
    std::string sessionId;
    SSL_SESSION* session;
    SSLSessionCacheManager* manager;
    folly::AsyncSSLSocket* sslSocket;
    std::unique_ptr<folly::DelayedDestruction::DestructorGuard> guard;
  };

  virtual ~SSLCacheProvider() = default;

  /**
   * Store a session in the external cache.
   * @param sessionId   Identifier that can be used later to fetch the
   *                      session with getFuture()
   * @param value       Serialized session to store
   * @param expiration  Relative expiration time: seconds from now
   * @return true if the storing of the session is initiated successfully
   *         (though not necessarily completed; the completion may
   *         happen either before or after this method returns), or
   *         false if the storing cannot be initiated due to an error.
   */
  virtual bool setAsync(
      const std::string& sessionId,
      const std::string& value,
      std::chrono::seconds expiration) = 0;

  /**
   * Retrieve a session from the external cache. Returns a future that will
   * hold the result of the request.
   * @param sessionId   Session ID to fetch
   * @return MultiFuture referring to the result of this request.
   */
  virtual folly::Future<folly::ssl::SSLSessionUniquePtr> getFuture(
      const std::string& sessionId) = 0;

  virtual void setSecrets(
      const std::vector<std::string>& /*oldSecrets*/,
      const std::vector<std::string>& /*currentSecrets*/,
      const std::vector<std::string>& /*newSecrets*/) {}
};

} // namespace wangle
