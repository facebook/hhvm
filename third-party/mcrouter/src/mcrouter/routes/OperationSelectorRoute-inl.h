/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <sstream>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeOperationSelectorRoute(
    carbon::RequestIdMap<
        typename RouterInfo::RoutableRequests,
        typename RouterInfo::RouteHandlePtr> operationPolicies,
    typename RouterInfo::RouteHandlePtr defaultPolicy) {
  return makeRouteHandleWithInfo<RouterInfo, OperationSelectorRoute>(
      std::move(operationPolicies), std::move(defaultPolicy));
}

} // namespace detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeOperationSelectorRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  if (!json.isObject()) {
    return factory.create(json);
  }

  auto jsonDefaultPolicy = json.get_ptr("default_policy");
  auto jOpPolicies = json.get_ptr("operation_policies");

  checkLogic(
      (jsonDefaultPolicy != nullptr || jOpPolicies != nullptr),
      "OperationSelectorRoute needs either 'default_policy' "
      "or 'operation_policies'");

  typename RouterInfo::RouteHandlePtr defaultPolicy;
  if (jsonDefaultPolicy) {
    defaultPolicy = factory.create(*jsonDefaultPolicy);
  }

  carbon::RequestIdMap<
      typename RouterInfo::RoutableRequests,
      typename RouterInfo::RouteHandlePtr>
      operationPolicies;
  if (jOpPolicies) {
    checkLogic(
        jOpPolicies->isObject(),
        "OperationSelectorRoute: operation_policies is not an object");

    std::map<std::string, const folly::dynamic*> orderedPolicies;
    for (auto& it : jOpPolicies->items()) {
      checkLogic(
          it.first.isString(),
          "OperationSelectorRoute: operation_policies' "
          "key is not a string");
      auto key = it.first.getString();
      orderedPolicies.emplace(std::move(key), &it.second);
    }

    // order is important here: named handles may not be resolved if we parse
    // policies in random order
    for (const auto& it : orderedPolicies) {
      auto id = carbon::getTypeIdByName(
          it.first.data(), typename RouterInfo::RoutableRequests());
      checkLogic(
          id != 0,
          "Unknown operation: {}, valid operations: {}",
          it.first,
          []() -> std::string {
            std::stringstream str;
            insertTypeIds(str, typename RouterInfo::RoutableRequests());
            return str.str();
          }());

      operationPolicies.set(id, factory.create(*it.second));
    }

    return detail::makeOperationSelectorRoute<RouterInfo>(
        std::move(operationPolicies), std::move(defaultPolicy));
  }

  return defaultPolicy;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
