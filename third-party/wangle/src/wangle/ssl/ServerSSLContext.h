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

#include <memory>
#include <string>

#include <folly/io/async/SSLContext.h>

namespace folly {

class EventBase;
class SocketAddress;

} // namespace folly

namespace wangle {

struct SSLCacheOptions;
struct SSLContextConfig;
class SSLStats;
class SSLSessionCacheManager;
class SSLCacheProvider;

// A SSL Context that owns a session cache and ticket key manager.
// It is used for server side SSL connections.
class ServerSSLContext : public folly::SSLContext {
 public:
  explicit ServerSSLContext(SSLVersion version = TLSv1_2);

  virtual ~ServerSSLContext() override = default;

  void setupSessionCache(
      const SSLContextConfig& ctxConfig,
      const SSLCacheOptions& cacheOptions,
      const std::shared_ptr<SSLCacheProvider>& externalCache,
      const std::string& sessionIdContext,
      SSLStats* stats);

  // Get the session cache manager that this context manages.
  SSLSessionCacheManager* getSessionCacheManager() {
    return sessionCacheManager_.get();
  }

 private:
  std::unique_ptr<SSLSessionCacheManager> sessionCacheManager_;
};

} // namespace wangle
