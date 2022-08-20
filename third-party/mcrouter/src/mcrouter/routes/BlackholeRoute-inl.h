/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

inline void validatePolicies(const folly::dynamic& jsonMap) {
  static std::unordered_set<std::string> kSupportedOps = {
      "equals", "not_equals"};

  for (const auto& fieldPolicies : jsonMap.items()) {
    checkLogic(
        fieldPolicies.second.isArray(),
        "BlackholeRoute: policies for \'{}\' must be in an array.",
        fieldPolicies.first);
    for (const auto& policy : fieldPolicies.second) {
      checkLogic(
          policy.isObject(),
          "BlackholeRoute: each policy in the array for \'{}\' must be an object.",
          fieldPolicies.first);

      checkLogic(
          policy.find("op") != policy.items().end(),
          "BlackholeRoute: policy object for \'{}\' must contain an \"op\" option",
          fieldPolicies.first);
      checkLogic(
          kSupportedOps.find(policy.find("op")->second.asString()) !=
              kSupportedOps.end(),
          "BlackholeRoute: policy op for \'{}\' is not supported",
          fieldPolicies.first);

      checkLogic(
          policy.find("value") != policy.items().end(),
          "BlackholeRoute: policy object for \'{}\' must contain an \"value\" option",
          fieldPolicies.first);
    }
  }
}
} // namespace detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeBlackholeRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "BlackholeRoute: config is not an object.");

  auto jDefault = json.get_ptr("default");
  checkLogic(
      jDefault != nullptr, "BlackholeRoute: 'default' property is missing.");
  auto defaultRh = factory.create(*jDefault);

  auto blackholeRh = createNullRoute<typename RouterInfo::RouteHandleIf>();
  if (auto jBlackhole = json.get_ptr("blackhole_child")) {
    blackholeRh = factory.create(*jBlackhole);
  }

  auto jPolicies = json.get_ptr("policies");
  checkLogic(jPolicies != nullptr, "BlackholeRoute must specify 'policies'");
  checkLogic(
      jPolicies->isObject(), "BlackholeRoute: 'policies' must be an object.");
  // validate policies
  detail::validatePolicies(*jPolicies);

  return makeRouteHandleWithInfo<RouterInfo, BlackholeRoute>(
      std::move(defaultRh), std::move(blackholeRh), *jPolicies);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
