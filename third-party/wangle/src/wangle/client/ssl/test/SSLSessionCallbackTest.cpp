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

#include <folly/io/async/SSLContext.h>
#include <folly/portability/GTest.h>
#include <wangle/client/ssl/SSLSessionCallbacks.h>
#include <wangle/client/ssl/test/TestUtil.h>
#include <map>
#include <vector>

using namespace wangle;

using folly::SSLContext;

// One time use cache for testing that uses size_t as the cache key
class FakeSessionCallbacks : public SSLSessionCallbacks {
 public:
  void setSSLSession(
      const std::string& key,
      folly::ssl::SSLSessionUniquePtr session) noexcept override {
    cache_.emplace(key, std::move(session));
  }

  folly::ssl::SSLSessionUniquePtr getSSLSession(
      const std::string& key) const noexcept override {
    auto it = cache_.find(key);
    if (it == cache_.end()) {
      return folly::ssl::SSLSessionUniquePtr(nullptr);
    }
    auto sess = std::move(it->second);
    cache_.erase(it);
    return sess;
  }

  bool removeSSLSession(const std::string&) noexcept override {
    return true;
  }

  size_t size() const override {
    return cache_.size();
  }

 private:
  mutable std::map<std::string, folly::ssl::SSLSessionUniquePtr> cache_;
};

TEST(SSLSessionCallbackTest, AttachMultiple) {
  SSLContext c1;
  SSLContext c2;
  FakeSessionCallbacks cb;
  FakeSessionCallbacks::attachCallbacksToContext(&c1, &cb);
  FakeSessionCallbacks::attachCallbacksToContext(&c2, &cb);

  auto cb1 = FakeSessionCallbacks::getCacheFromContext(c1.getSSLCtx());
  auto cb2 = FakeSessionCallbacks::getCacheFromContext(c2.getSSLCtx());
  EXPECT_EQ(cb1, cb2);

  FakeSessionCallbacks::detachCallbacksFromContext(&c1, cb1);
  EXPECT_FALSE(FakeSessionCallbacks::getCacheFromContext(c1.getSSLCtx()));

  FakeSessionCallbacks unused;
  FakeSessionCallbacks::detachCallbacksFromContext(&c2, &unused);
  cb2 = FakeSessionCallbacks::getCacheFromContext(c2.getSSLCtx());
  EXPECT_EQ(&cb, cb2);
}
