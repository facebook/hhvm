/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/client/SynchronizedLruPskCache.h>
#include <fizz/client/test/Utilities.h>
#include <folly/Format.h>

using namespace testing;

namespace fizz {
namespace client {
namespace test {

class SynchronizedLruPskCacheTest : public Test {
 public:
  void SetUp() override {
    cache_ = std::make_unique<SynchronizedLruPskCache>(3);
    ticketTime_ = std::chrono::system_clock::now();
  }

 protected:
  CachedPsk getCachedPsk(std::string pskName = "PSK") {
    return getTestPsk(pskName, ticketTime_);
  }

  std::unique_ptr<SynchronizedLruPskCache> cache_;
  std::chrono::system_clock::time_point ticketTime_;
};

TEST_F(SynchronizedLruPskCacheTest, TestBasic) {
  auto psk = getCachedPsk();
  cache_->putPsk("fizz", psk);
  auto cachedPsk = cache_->getPsk("fizz");
  EXPECT_TRUE(cachedPsk);
  pskEq(psk, *cachedPsk);

  cache_->removePsk("fizz");
  EXPECT_FALSE(cache_->getPsk("fizz"));
}

TEST_F(SynchronizedLruPskCacheTest, TestEviction) {
  for (int i : {1, 2, 3}) {
    auto pskName = folly::sformat("psk {}", i);
    auto psk = getCachedPsk(pskName);
    cache_->putPsk(pskName, psk);
  }

  // Prime 1 to be evicted
  cache_->getPsk("psk 2");
  cache_->getPsk("psk 3");

  auto evictingPsk = getCachedPsk("psk 4");
  cache_->putPsk("psk 4", evictingPsk);

  auto psk1 = cache_->getPsk("psk 1");
  EXPECT_FALSE(psk1);
}

TEST_F(SynchronizedLruPskCacheTest, TestExpiredGet) {
  auto pskName = "let_it_expire_psk";
  auto psk = getCachedPsk(pskName);
  psk.ticketExpirationTime =
      std::chrono::system_clock::now() - std::chrono::seconds(10);
  cache_->putPsk(pskName, psk);
  auto expired_psk = cache_->getPsk(pskName);
  EXPECT_FALSE(expired_psk);
}

} // namespace test
} // namespace client
} // namespace fizz
