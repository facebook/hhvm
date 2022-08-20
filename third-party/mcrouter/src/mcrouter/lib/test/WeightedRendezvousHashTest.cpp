/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>

#include <gtest/gtest.h>

#include <folly/Conv.h>

#include "mcrouter/lib/WeightedRendezvousHashFunc.h"
#include "mcrouter/lib/test/HashTestUtil.h"

using namespace facebook::memcache;

namespace facebook {
namespace memcache {
namespace test {

TEST(WeightedRendezvousHashFunc, basic) {
  auto endpoints3 = genEndpoints(3);
  std::vector<double> weights3(3, 1);
  auto jWeight3 = genWeights(weights3);
  auto func3 = WeightedRendezvousHashFunc(endpoints3.second, jWeight3);

  EXPECT_EQ(func3("sample"), 0);
  EXPECT_EQ(func3(""), 2);
  EXPECT_EQ(func3("mykey"), 1);

  std::string testMaxKey;
  //-128 .. 127
  for (int i = 0; i < 256; ++i) {
    testMaxKey.push_back(i - 128);
  }
  EXPECT_EQ(func3(testMaxKey), 1);

  auto endpoints343 = genEndpoints(343);
  std::vector<double> weights343(343, 1);
  auto jWeight343 = genWeights(weights343);
  auto func343 = WeightedRendezvousHashFunc(endpoints343.second, jWeight343);

  EXPECT_EQ(func343(testMaxKey), 142);
  EXPECT_EQ(func343("sample"), 115);
  EXPECT_EQ(func343(""), 279);
  EXPECT_EQ(func343("mykey"), 302);
}

TEST(WeightedRendezvousHashFunc, rendezvous_3) {
  // 3 endpoints with same weights.
  auto endpoints3 = genEndpoints(3);
  std::vector<double> weights3(3, 1);
  auto jWeight3 = genWeights(weights3);
  auto rendezvous3 = WeightedRendezvousHashFunc(endpoints3.second, jWeight3);

  std::vector<size_t> rendezvousCounts(3, 0);
  for (size_t i = 0; i < 10000; ++i) {
    auto key = "mykey:" + folly::to<std::string>(i);
    ++rendezvousCounts[rendezvous3(key)];
  }
  EXPECT_EQ(rendezvousCounts, std::vector<size_t>({3409, 3360, 3231}));

  // New set of weights.
  weights3 = std::vector<double>{1, 0.5, 0.1};
  jWeight3 = genWeights(weights3);
  rendezvous3 = WeightedRendezvousHashFunc(endpoints3.second, jWeight3);

  rendezvousCounts = std::vector<size_t>(3, 0);
  for (size_t i = 0; i < 10000; ++i) {
    auto key = "mykey:" + folly::to<std::string>(i);
    ++rendezvousCounts[rendezvous3(key)];
  }
  EXPECT_EQ(rendezvousCounts, std::vector<size_t>({6292, 3118, 590}));

  // New set of weights.
  weights3 = std::vector<double>{1, 0, 0.5};
  jWeight3 = genWeights(weights3);
  rendezvous3 = WeightedRendezvousHashFunc(endpoints3.second, jWeight3);

  rendezvousCounts = std::vector<size_t>(3, 0);
  for (size_t i = 0; i < 10000; ++i) {
    auto key = "mykey:" + folly::to<std::string>(i);
    ++rendezvousCounts[rendezvous3(key)];
  }
  EXPECT_EQ(rendezvousCounts, std::vector<size_t>({6737, 0, 3263}));
}

TEST(WeightedRendezvousHashFunc, rendezvous_10) {
  // 10 endpoints with different weights.
  auto endpoints10 = genEndpoints(10);
  std::vector<double> weights10{1, 1, 1, 1, 1, 0.5, 0.5, 0.5, 0.5, 0.5};
  auto jWeight10 = genWeights(weights10);
  auto rendezvous10 = WeightedRendezvousHashFunc(endpoints10.second, jWeight10);

  std::vector<size_t> rendezvousCounts(10, 0);
  for (size_t i = 0; i < 10000; ++i) {
    auto key = "mykey:" + folly::to<std::string>(i);
    ++rendezvousCounts[rendezvous10(key)];
  }

  EXPECT_EQ(
      rendezvousCounts,
      std::vector<size_t>(
          {1381, 1299, 1283, 1359, 1296, 667, 669, 665, 672, 709}));
}

TEST(WeightedRendezvousHashFunc, rendezvous_rehash) {
  const uint32_t n = 499;
  auto combined = genEndpoints(n);
  const auto& endpoints = combined.second;
  std::vector<double> weights(n, 1);
  auto jWeights = genWeights(weights);
  auto rendezvous = WeightedRendezvousHashFunc(endpoints, jWeights);

  // Number of rehashes if we remove one element
  auto removeCompare = [&](std::vector<folly::StringPiece>& newEndpoints,
                           std::vector<folly::StringPiece>::iterator it) {
    newEndpoints.erase(it);
    std::vector<double> newWeights(newEndpoints.size(), 1);
    auto newJWeights = genWeights(newWeights);
    auto newRendezvous = WeightedRendezvousHashFunc(newEndpoints, newJWeights);

    int numDiff = 0;
    for (size_t i = 0; i < 10000; ++i) {
      auto key = "mykey:" + folly::to<std::string>(i);
      if (endpoints[rendezvous(key)] != newEndpoints[newRendezvous(key)]) {
        ++numDiff;
      }
    }

    return numDiff;
  };

  auto frontRemoved = endpoints;
  EXPECT_EQ(removeCompare(frontRemoved, frontRemoved.begin()), 19);

  auto backRemoved = endpoints;
  EXPECT_EQ(removeCompare(backRemoved, backRemoved.end() - 1), 17);

  auto midRemoved = endpoints;
  EXPECT_EQ(removeCompare(midRemoved, midRemoved.begin() + n / 2), 21);
}

} // namespace test
} // namespace memcache
} // namespace facebook
