/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <folly/Range.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/Trie.h"
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * This class contains RouteHandles that should be used depending on key
 * prefix.
 */
template <class RouteHandleIf>
class PrefixSelectorRoute {
 public:
  /// Trie that acts like map from key prefix to corresponding RouteHandle.
  Trie<std::shared_ptr<RouteHandleIf>> policies;
  /// Used when no RouteHandle found in policies
  std::shared_ptr<RouteHandleIf> wildcard;

  PrefixSelectorRoute() = default;

  // This should be RouteHandleFactory<RouteHandleIf> but we mock it in tests.
  template <typename Factory>
  PrefixSelectorRoute(Factory& factory, const folly::dynamic& json) {
    // if json is not PrefixSelectorRoute, just treat it as a wildcard.
    if (!json.isObject() || !json.count("type") || !json["type"].isString() ||
        json["type"].stringPiece() != "PrefixSelectorRoute") {
      wildcard = factory.create(json);
      return;
    }

    auto jPolicies = json.get_ptr("policies");
    auto jWildcard = json.get_ptr("wildcard");
    checkLogic(
        jPolicies || jWildcard, "PrefixSelectorRoute: no policies/wildcard");
    if (jPolicies) {
      checkLogic(
          jPolicies->isObject(),
          "PrefixSelectorRoute: policies is not an object");
      std::vector<std::pair<folly::StringPiece, const folly::dynamic*>> items;
      for (const auto& it : jPolicies->items()) {
        checkLogic(
            it.first.isString(),
            "PrefixSelectorRoute: policy key is not a string");
        items.emplace_back(it.first.stringPiece(), &it.second);
      }
      // order is important
      std::sort(items.begin(), items.end());
      for (const auto& it : items) {
        policies.emplace(it.first, factory.create(*it.second));
      }
    }

    if (jWildcard) {
      wildcard = factory.create(*jWildcard);
    }
  }
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
