/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/PrefixSelectorRoute.h"

#include <folly/dynamic.h>
#include <gtest/gtest.h>

namespace facebook::memcache::mcrouter {
namespace {

using RouteHandle = int;

struct MockFactory {
  std::shared_ptr<RouteHandle> create(const folly::dynamic& obj) const {
    const folly::dynamic* val = obj.isObject() ? obj.get_ptr("val") : nullptr;
    if (val && !val->isInt()) {
      val = nullptr;
    }
    return std::make_shared<int>(val ? val->getInt() : -1);
  }
};

struct ExpectedPrefixSelectorRoute {
  RouteHandle wildcard = -1;
  std::vector<std::pair<std::string, RouteHandle>> policies;
};

void constructionTest(
    std::string_view input,
    ExpectedPrefixSelectorRoute expected) {
  MockFactory factory;
  PrefixSelectorRoute<RouteHandle> route(factory, folly::parseJson(input));

  auto toInt = [](const std::shared_ptr<RouteHandle>& rh) -> int {
    return rh ? *rh : -1;
  };
  std::vector<std::pair<std::string, RouteHandle>> policies;
  for (const auto& p : route.policies) {
    policies.emplace_back(std::string(p.first), toInt(p.second));
  }

  EXPECT_EQ(expected.wildcard, toInt(route.wildcard));
  EXPECT_EQ(expected.policies, policies);
}

TEST(PrefixSelectorRouteTest, Construction) {
  constructionTest(R"_({})_", {});
  constructionTest(R"_([])_", {});
  constructionTest(R"_("")_", {});
  // clang-format off
  constructionTest(R"_({
    "type": "PrefixSelectorRoute",
    "wildcard" : { "val": 42 }
    })_",
    {.wildcard = 42});
  constructionTest(R"_({ "val": 42 })_",
    {.wildcard = 42});
  constructionTest(R"_({
    "type": "PrefixSelectorRoute",
    "policies" : {}
    })_",
    {});
  constructionTest(R"_({
    "type": "PrefixSelectorRoute",
    "policies" : {}
    })_",
    {});
  constructionTest(R"_({
    "type": "PrefixSelectorRoute",
    "policies" : {
        "a" : { "val": 42 },
        "b" : { "val": 43 }
    }
    })_",
    {
        .policies = {{"a", 42}, {"b", 43}},
    });
  constructionTest(R"_({
    "type": "PrefixSelectorRoute",
    "policies" : {
        "b" : { "val": 43 },
        "a" : { "val": 42 }
    }
    })_",
    {
        .policies = {{"a", 42}, {"b", 43}},
    });
  constructionTest(R"_({
    "type": "PrefixSelectorRoute",
    "policies" : {
        "a4" : { "val": 4 },
        "a3" : { "val": 3 },
        "a2" : { "val": 2 },
        "a1" : { "val": 1 },
        "a0" : { "val": 0 }
    },
    "wildcard" : { "val": 42 }
    })_",
    {
        .wildcard = 42,
        .policies = {{"a0", 0}, {"a1", 1}, {"a2", 2}, {"a3", 3}, {"a4", 4}},
    });
  // clang-format on
}

} // namespace
} // namespace facebook::memcache::mcrouter
