/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Memory.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/utils/test/MockTime.h>

#include "proxygen/lib/dns/test/Dummies.h"

using namespace proxygen;
using namespace std;
using namespace testing;

struct CachingDNSParams {
  size_t cachingMaxSize = 4096;
  size_t cachingClearSize = 1;
  size_t staleCacheSizeMultiplier = 4;
  size_t staleCacheTTLMin = 24 * 60 * 60;
  size_t staleCacheTTLScale = 3;
};

class CachingDNSResolverFixture : public testing::Test {
 public:
  void SetUp() override {
    initializeWithDefaultParams();
  }

  void initializeResolverWithParams(const CachingDNSParams& params) {
    DNSResolver::UniquePtr p(new DummyDNSResolver());
    auto timeUtil = std::make_unique<MockTimeUtil>();
    underlyingResolver_ = dynamic_cast<DummyDNSResolver*>(p.get());
    CHECK(underlyingResolver_);
    timeUtil_ = timeUtil.get();

    cachingResolver_ =
        std::make_unique<CachingDNSResolver>(std::move(p),
                                             params.cachingMaxSize,
                                             params.cachingClearSize,
                                             params.staleCacheSizeMultiplier,
                                             params.staleCacheTTLMin,
                                             params.staleCacheTTLScale,
                                             std::move(timeUtil));
  }

  void initializeWithDefaultParams() {
    initializeResolverWithParams(CachingDNSParams{});
  }

 protected:
  std::unique_ptr<CachingDNSResolver> cachingResolver_;
  DummyDNSResolver* underlyingResolver_{nullptr};
  MockTimeUtil* timeUtil_{nullptr};
  DummyDNSClient cb_;
};

TEST_F(CachingDNSResolverFixture, NoCrashWhileDestructing) {
  string hostname("foo.bar.com");
  cachingResolver_->resolveHostname(&cb_, hostname);
  cachingResolver_.reset(); // no crash and error during deconstructing
}

TEST_F(CachingDNSResolverFixture, ResolutionError) {
  underlyingResolver_->setIsRunning(false);
  cachingResolver_->resolveHostname(&cb_, "should fail");
  EXPECT_EQ(cb_.getNumFailures(), 1);
}

TEST_F(CachingDNSResolverFixture, CacheHit) {
  string hostname("foo.bar.com");
  cachingResolver_->resolveHostname(&cb_, hostname);
  // Underlying resolver should have been hit.
  EXPECT_EQ(underlyingResolver_->getHitCount(), 1);

  cachingResolver_->resolveHostname(&cb_, hostname);
  EXPECT_EQ(underlyingResolver_->getHitCount(), 1);

  EXPECT_EQ(cb_.getNumSuccesses(), 2);
}

TEST_F(CachingDNSResolverFixture, CacheClearAfterMax) {
  initializeResolverWithParams(
      CachingDNSParams{.cachingMaxSize = 5, .cachingClearSize = 2});

  // Pre-fill the cache with 5 records.
  for (int i = 0; i < 5; i++) {
    cachingResolver_->resolveHostname(&cb_, folly::to<std::string>(i));
  }
  // Should evict the first two records.
  cachingResolver_->resolveHostname(&cb_, "6");
  EXPECT_EQ(underlyingResolver_->getHitCount(), 6);

  cachingResolver_->resolveHostname(&cb_, "0");
  EXPECT_EQ(underlyingResolver_->getHitCount(), 7);
  cachingResolver_->resolveHostname(&cb_, "1");
  EXPECT_EQ(underlyingResolver_->getHitCount(), 8);
}

TEST_F(CachingDNSResolverFixture, CacheClearExpiredRecords) {
  // Magical string that forces Dummies to have a TTL of 2.
  string hostname("foo");
  cachingResolver_->resolveHostname(&cb_, hostname);
  EXPECT_EQ(underlyingResolver_->getHitCount(), 1);

  // Should hit cache.
  cachingResolver_->resolveHostname(&cb_, hostname);
  EXPECT_EQ(underlyingResolver_->getHitCount(), 1);

  timeUtil_->advance(std::chrono::milliseconds(5000));

  // Should be a partial miss.
  cachingResolver_->resolveHostname(&cb_, hostname);
  EXPECT_EQ(underlyingResolver_->getHitCount(), 2);
}

TEST_F(CachingDNSResolverFixture, StaleCacheHitAfterFailure) {
  // Magical string that forces Dummies to have a TTL of 2.
  string hostname("foo");
  cachingResolver_->resolveHostname(&cb_, hostname);
  EXPECT_EQ(underlyingResolver_->getHitCount(), 1);

  timeUtil_->advance(std::chrono::milliseconds(5000));

  underlyingResolver_->setIsRunning(false);
  // Should be a hit from stale cache.
  cachingResolver_->resolveHostname(&cb_, hostname);
  EXPECT_EQ(underlyingResolver_->getHitCount(), 1);
  EXPECT_EQ(cb_.getNumSuccesses(), 2);
}
