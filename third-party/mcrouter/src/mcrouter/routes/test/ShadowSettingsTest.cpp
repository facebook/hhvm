/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <gtest/gtest.h>

#include <folly/Range.h>
#include <folly/json/json.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/ShadowSettings.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

class ShadowSettingsTest : public ::testing::Test {
 public:
  template <class RouterInfo>
  CarbonRouterInstance<RouterInfo>& getRouter() const {
    constexpr folly::StringPiece kRouterInfoName(RouterInfo::name);
    const std::string kInstanceName =
        folly::to<std::string>("TestRouter:", kRouterInfoName);
    auto router =
        CarbonRouterInstance<RouterInfo>::init(kInstanceName, getOpts());
    CHECK(router != nullptr) << "router shouldn't be nullptr";
    return *router;
  }

  void expectApproximatelyEqual(size_t expected, size_t actual, size_t margin) {
    EXPECT_TRUE(actual >= (expected - margin));
    EXPECT_TRUE(actual <= (expected + margin));
  }

  std::mt19937& randomGenerator() {
    return randomGenerator_;
  }

 private:
  static McrouterOptions getOpts() {
    // Dummy config, used just to spin up mcrouter.
    constexpr folly::StringPiece kDummyConfig = R"(
      {
        "route": "NullRoute"
      }
      )";
    McrouterOptions opts;
    opts.num_proxies = 1;
    opts.stats_logging_interval = 0;
    opts.config = kDummyConfig.str();
    return opts;
  }

  std::mt19937 randomGenerator_;
};

TEST_F(ShadowSettingsTest, create) {
  constexpr folly::StringPiece kConfig = R"(
  {
    "key_fraction_range": [0.5, 1.0],
    "index_range": [0, 1]
  }
  )";

  const auto json = folly::parseJson(kConfig);
  auto& router = getRouter<MemcacheRouterInfo>();

  auto shadowSettings = ShadowSettings::create(json, router);
  EXPECT_TRUE(shadowSettings != nullptr);
}

TEST_F(ShadowSettingsTest, shouldRoute) {
  constexpr folly::StringPiece kConfig = R"(
  {
    "key_fraction_range": [0.0, 0.5]
  }
  )";

  const auto json = folly::parseJson(kConfig);
  auto& router = getRouter<MemcacheRouterInfo>();

  auto shadowSettings = ShadowSettings::create(json, router);
  ASSERT_TRUE(shadowSettings != nullptr);

  McGetRequest req1("good_key");
  bool res1 =
      shadowSettings->shouldShadow(req1, std::nullopt, randomGenerator());
  EXPECT_TRUE(res1);

  McGetRequest req2("out_of_range_key_test");
  bool res2 =
      shadowSettings->shouldShadow(req2, std::nullopt, randomGenerator());
  EXPECT_FALSE(res2);

  constexpr size_t kNumRuns = 10000;
  constexpr size_t kExpected = kNumRuns / 2;
  constexpr size_t kMargin = 10000 * 0.01;

  size_t yes = 0;
  size_t no = 0;
  for (size_t i = 0; i < kNumRuns; ++i) {
    McGetRequest req(folly::to<std::string>(i));
    if (shadowSettings->shouldShadow(req, std::nullopt, randomGenerator())) {
      ++yes;
    } else {
      ++no;
    }
  }

  expectApproximatelyEqual(kExpected, yes, kMargin);
  expectApproximatelyEqual(kExpected, no, kMargin);
}

TEST_F(ShadowSettingsTest, shouldRoute_random) {
  constexpr folly::StringPiece kConfig = R"(
  {
    "key_fraction_range": [0.0, 1.0],
    "requests_fraction": 0.5
  }
  )";

  const auto json = folly::parseJson(kConfig);
  auto& router = getRouter<MemcacheRouterInfo>();

  auto shadowSettings = ShadowSettings::create(json, router);
  ASSERT_TRUE(shadowSettings != nullptr);

  constexpr size_t kNumRuns = 10000;
  constexpr size_t kExpected = kNumRuns / 2;
  constexpr size_t kMargin = 10000 * 0.01;

  size_t yes = 0;
  size_t no = 0;
  for (size_t i = 0; i < kNumRuns; ++i) {
    McGetRequest req(folly::to<std::string>(i));
    if (shadowSettings->shouldShadow(req, std::nullopt, randomGenerator())) {
      ++yes;
    } else {
      ++no;
    }
  }

  expectApproximatelyEqual(kExpected, yes, kMargin);
  expectApproximatelyEqual(kExpected, no, kMargin);
}

TEST_F(ShadowSettingsTest, shouldRouteByBucket) {
  constexpr folly::StringPiece kConfig = R"(
  {
    "key_fraction_range": [0.2, 0.6]
  }
  )";

  const auto json = folly::parseJson(kConfig);
  auto& router = getRouter<MemcacheRouterInfo>();

  auto shadowSettings = ShadowSettings::create(json, router, 100);
  ASSERT_TRUE(shadowSettings != nullptr);
  auto bucketRange = shadowSettings->bucketRange();
  EXPECT_EQ(bucketRange.start, 19);
  EXPECT_EQ(bucketRange.end, 59);

  McGetRequest req("test_key");
  for (int i = 0; i < 120; i++) {
    EXPECT_EQ(
        shadowSettings->shouldShadow(req, i, randomGenerator()),
        i >= 19 && i <= 59);
  }
}
