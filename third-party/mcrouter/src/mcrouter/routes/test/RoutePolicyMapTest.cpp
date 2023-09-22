/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/RoutePolicyMap.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "mcrouter/routes/PrefixSelectorRoute.h"

namespace facebook::memcache::mcrouter {
namespace {

using RouteHandle = int;
using SharedSelector = std::shared_ptr<PrefixSelectorRoute<RouteHandle>>;

const auto kRouteHandles = [] {
  std::vector<std::shared_ptr<RouteHandle>> res;
  res.reserve(1000);
  for (int i = 0; i != 1000; ++i) {
    res.push_back(std::make_shared<RouteHandle>(i));
  }
  return res;
}();

struct MockPrefixSelectorRoute {
  std::optional<RouteHandle> wildcard;
  std::vector<std::pair<std::string, RouteHandle>> policies;

  /*implicit*/ operator SharedSelector() const {
    auto res = std::make_shared<PrefixSelectorRoute<RouteHandle>>();

    if (wildcard) {
      res->wildcard = kRouteHandles[*wildcard];
    }

    for (const auto& [k, v] : policies) {
      res->policies.emplace(k, kRouteHandles[v]);
    }

    return res;
  }
};

struct MapsToTest {
  explicit MapsToTest(const std::vector<SharedSelector>& clusters)
      : m0(clusters, false), m1(clusters, true), m2(clusters) {}

  RoutePolicyMap<RouteHandle> m0;
  RoutePolicyMap<RouteHandle> m1;
  RoutePolicyMapV2<RouteHandle> m2;
};

MapsToTest makeMap(const std::vector<MockPrefixSelectorRoute>& routes) {
  std::vector<SharedSelector> clusters(routes.begin(), routes.end());
  return MapsToTest(clusters);
}

std::vector<int> routesFor(MapsToTest& maps, folly::StringPiece key) {
  auto resForMap = [&](const auto& m) {
    std::vector<int> res;
    auto actual = m.getTargetsForKey(key);
    res.reserve(actual.size());
    for (const auto& x : actual) {
      EXPECT_NE(x, nullptr);
      res.push_back(x != nullptr ? *x : -1);
    }
    return res;
  };

  auto r0 = resForMap(maps.m0);
  auto r1 = resForMap(maps.m1);
  auto r2 = resForMap(maps.m2);
  EXPECT_EQ(r0, r1) << key;
  EXPECT_EQ(r0, r2) << key;
  return r0;
}

TEST(RoutePolicyMapTest, NoPolicies) {
  auto m = makeMap({});
  ASSERT_THAT(routesFor(m, ""), ::testing::ElementsAre());
  ASSERT_THAT(routesFor(m, "a"), ::testing::ElementsAre());
}

TEST(RoutePolicyMapTest, OnePolicyNoWildCard) {
  auto m = makeMap({{.policies = {{"a", 1}}}});

  ASSERT_THAT(routesFor(m, ""), ::testing::ElementsAre());
  ASSERT_THAT(routesFor(m, "a"), ::testing::ElementsAre(1));
  ASSERT_THAT(routesFor(m, "b"), ::testing::ElementsAre());
}

TEST(RoutePolicyMapTest, OnePolicy) {
  auto m =
      makeMap({{.wildcard = 3, .policies = {{"a", 1}, {"ab", 2}, {"b", 3}}}});

  ASSERT_THAT(routesFor(m, ""), ::testing::ElementsAre(3));
  ASSERT_THAT(routesFor(m, "o"), ::testing::ElementsAre(3));

  ASSERT_THAT(routesFor(m, "a"), ::testing::ElementsAre(1));
  ASSERT_THAT(routesFor(m, "ab"), ::testing::ElementsAre(2));
  ASSERT_THAT(routesFor(m, "ac"), ::testing::ElementsAre(1));
  ASSERT_THAT(routesFor(m, "abc"), ::testing::ElementsAre(2));
  ASSERT_THAT(routesFor(m, "b"), ::testing::ElementsAre(3));
}

TEST(RoutePolicyMapTest, WildCardOverwritten) {
  auto m = makeMap({{.wildcard = 1, .policies = {{"", 2}, {"a", 3}}}});

  ASSERT_THAT(routesFor(m, ""), ::testing::ElementsAre(2));
  ASSERT_THAT(routesFor(m, "a"), ::testing::ElementsAre(3));
  ASSERT_THAT(routesFor(m, "b"), ::testing::ElementsAre(2));
}

TEST(RoutePolicyMapTest, TwoPolicies) {
  auto m = makeMap(
      {{.wildcard = 10, .policies = {{"a", 1}, {"bb", 2}}},
       {.wildcard = 20,
        .policies = {{"a", 1}, {"aa", 5}, {"b", 6}, {"bc", 8}}}});

  ASSERT_THAT(routesFor(m, ""), ::testing::ElementsAre(10, 20));
  ASSERT_THAT(routesFor(m, "a"), ::testing::ElementsAre(1));
  ASSERT_THAT(routesFor(m, "aa"), ::testing::ElementsAre(1, 5));
  ASSERT_THAT(routesFor(m, "b"), ::testing::ElementsAre(10, 6));
  ASSERT_THAT(routesFor(m, "bb"), ::testing::ElementsAre(2, 6));
  ASSERT_THAT(routesFor(m, "bc"), ::testing::ElementsAre(10, 8));
}

TEST(RoutePolicyMapTest, RepeatedWildcard) {
  auto m = makeMap(
      {{.wildcard = 10, .policies = {{"a", 1}}},
       {.wildcard = 10, .policies = {{"a", 2}}}});
  ASSERT_THAT(routesFor(m, "b"), ::testing::ElementsAre(10));
  ASSERT_THAT(routesFor(m, "a"), ::testing::ElementsAre(1, 2));
}

} // namespace
} // namespace facebook::memcache::mcrouter
