/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeRoutingGroupRoute(
    std::map<std::string, typename RouterInfo::RouteHandlePtr>
        routingGroupPolicies,
    typename RouterInfo::RouteHandlePtr defaultPolicy) {
  return makeRouteHandleWithInfo<RouterInfo, RoutingGroupRoute>(
      std::move(routingGroupPolicies), std::move(defaultPolicy));
}

} // namespace detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeRoutingGroupRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  if (!json.isObject()) {
    return factory.create(json);
  }

  auto jsonDefaultPolicy = json.get_ptr("default_policy");
  auto jOpPolicies = json.get_ptr("routing_group_policies");

  checkLogic(
      (jsonDefaultPolicy != nullptr || jOpPolicies != nullptr),
      "RoutingGroupRoute needs either 'default_policy' "
      "or 'routing_group_policies'");

  typename RouterInfo::RouteHandlePtr defaultPolicy;
  if (jsonDefaultPolicy) {
    defaultPolicy = factory.create(*jsonDefaultPolicy);
  }

  std::map<std::string, typename RouterInfo::RouteHandlePtr>
      routingGroupPolicies;

  if (jOpPolicies) {
    checkLogic(
        jOpPolicies->isObject(),
        "RoutingGroupRoute: routing_group_policies is not an object");

    for (auto& it : jOpPolicies->items()) {
      checkLogic(
          it.first.isString(),
          "RoutingGroupRoute: routing_group_policies' "
          "key is not a string");

      auto key = it.first.getString();
      checkLogic(
          key == carbon::kArithmeticKey || key == carbon::kDeleteKey ||
              key == carbon::kGetKey || key == carbon::kUpdateKey,
          "RoutingGroupRoute: {} is not a valid routing group.",
          key);

      checkLogic(
          routingGroupPolicies.find(key) == routingGroupPolicies.end(),
          "RoutingGroupRoute: {} is a duplicate key!",
          key);

      routingGroupPolicies[key] = factory.create(it.second);
    }

    return detail::makeRoutingGroupRoute<RouterInfo>(
        std::move(routingGroupPolicies), std::move(defaultPolicy));
  }

  return defaultPolicy;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
