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

#include <wangle/client/ssl/ThreadSafeSSLSessionCache.h>

using folly::SharedMutex;

namespace wangle {

void ThreadSafeSSLSessionCache::setSSLSession(
    const std::string& identity,
    folly::ssl::SSLSessionUniquePtr session) noexcept {
  std::unique_lock lock(mutex_);
  delegate_->setSSLSession(identity, std::move(session));
}

folly::ssl::SSLSessionUniquePtr ThreadSafeSSLSessionCache::getSSLSession(
    const std::string& identity) const noexcept {
  std::shared_lock lock(mutex_);
  return delegate_->getSSLSession(identity);
}

bool ThreadSafeSSLSessionCache::removeSSLSession(
    const std::string& identity) noexcept {
  std::unique_lock lock(mutex_);
  return delegate_->removeSSLSession(identity);
}

bool ThreadSafeSSLSessionCache::supportsPersistence() const noexcept {
  std::shared_lock lock(mutex_);
  return delegate_->supportsPersistence();
}

size_t ThreadSafeSSLSessionCache::size() const {
  std::shared_lock lock(mutex_);
  return delegate_->size();
}

} // namespace wangle
