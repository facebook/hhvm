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

#include <folly/ssl/OpenSSLPtrTypes.h>
#include <wangle/client/ssl/SSLSessionCallbacks.h>

#include <folly/SharedMutex.h>

namespace wangle {

/**
 * A SSL session cache that can be used safely across threads.
 * This is useful for clients who cannot avoid sharing the cache
 * across threads. It uses a read/write lock for efficiency.
 */
class ThreadSafeSSLSessionCache : public SSLSessionCallbacks {
 public:
  explicit ThreadSafeSSLSessionCache(
      std::unique_ptr<SSLSessionCallbacks> delegate)
      : delegate_(std::move(delegate)) {}

  // From SSLSessionCallbacks
  void setSSLSession(
      const std::string& identity,
      folly::ssl::SSLSessionUniquePtr session) noexcept override;
  folly::ssl::SSLSessionUniquePtr getSSLSession(
      const std::string& identity) const noexcept override;
  bool removeSSLSession(const std::string& identity) noexcept override;
  bool supportsPersistence() const noexcept override;
  size_t size() const override;

 private:
  std::unique_ptr<SSLSessionCallbacks> delegate_;
  mutable folly::SharedMutex mutex_;
};

} // namespace wangle
