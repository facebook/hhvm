/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>

#include <gtest/gtest.h>

#include <folly/Conv.h>

#include "mcrouter/lib/RendezvousHashFunc.h"
#include "mcrouter/lib/test/HashTestUtil.h"

using namespace facebook::memcache;

namespace {
RendezvousHashFunc genRendezvousHashFunc(int n) {
  auto combined = test::genEndpoints(n);
  return RendezvousHashFunc(combined.second, folly::dynamic());
}

} // namespace

TEST(RendezvousHashFunc, basic) {
  auto func_3 = genRendezvousHashFunc(3);

  EXPECT_EQ(func_3("sample"), 1);
  EXPECT_EQ(func_3(""), 1);
  EXPECT_EQ(func_3("mykey"), 1);

  std::string test_max_key;
  //-128 .. 127
  for (int i = 0; i < 256; ++i) {
    test_max_key.push_back(i - 128);
  }
  EXPECT_EQ(func_3(test_max_key), 1);

  auto func_343 = genRendezvousHashFunc(343);

  EXPECT_EQ(func_343(test_max_key), 183);
  EXPECT_EQ(func_343("sample"), 45);
  EXPECT_EQ(func_343(""), 291);
  EXPECT_EQ(func_343("mykey"), 132);
}

TEST(RendezvousHashFunc, rendezvous_3) {
  auto rendezvous_3 = genRendezvousHashFunc(3);

  std::vector<size_t> rendezvous_counts(3, 0);
  for (size_t i = 0; i < 1000; ++i) {
    auto key = "mykey:" + folly::to<std::string>(i);
    ++rendezvous_counts[rendezvous_3(key)];
  }

  EXPECT_EQ(rendezvous_counts, std::vector<size_t>({337, 353, 310}));
}

TEST(RendezvousHashFunc, rendezvous_10) {
  auto rendezvous_10 = genRendezvousHashFunc(10);

  std::vector<size_t> rendezvous_counts(10, 0);
  for (size_t i = 0; i < 10000; ++i) {
    auto key = "mykey:" + folly::to<std::string>(i);
    ++rendezvous_counts[rendezvous_10(key)];
  }

  EXPECT_EQ(
      rendezvous_counts,
      std::vector<size_t>(
          {947, 1026, 1028, 981, 1016, 970, 1013, 939, 1023, 1057}));
}

TEST(RendezvousHashFunc, rendezvous_rehash) {
  const uint32_t n = 499;
  auto combined = test::genEndpoints(n);
  const auto& endpoints = combined.second;

  RendezvousHashFunc rendezvous(endpoints, folly::dynamic());

  // Number of rehashes if we remove one element
  auto removeCompare = [&](std::vector<folly::StringPiece>& newEndpoints,
                           std::vector<folly::StringPiece>::iterator it) {
    newEndpoints.erase(it);

    RendezvousHashFunc newRendezvous(newEndpoints, folly::dynamic());

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
  EXPECT_EQ(removeCompare(frontRemoved, frontRemoved.begin()), 14);

  auto backRemoved = endpoints;
  EXPECT_EQ(removeCompare(backRemoved, backRemoved.end() - 1), 24);

  auto midRemoved = endpoints;
  EXPECT_EQ(removeCompare(midRemoved, midRemoved.begin() + n / 2), 15);
}
