/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/json/dynamic.h>

#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/routes/AllFastestRoute.h"
#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeAllFastestRoute(
    std::vector<typename RouterInfo::RouteHandlePtr> rh) {
  if (rh.empty()) {
    return createNullRoute<typename RouterInfo::RouteHandleIf>();
  }

  if (rh.size() == 1) {
    return std::move(rh[0]);
  }

  return makeRouteHandle<typename RouterInfo::RouteHandleIf, AllFastestRoute>(
      std::move(rh));
}

} // namespace detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeAllFastestRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  std::vector<typename RouterInfo::RouteHandlePtr> children;
  if (json.isObject()) {
    if (auto jchildren = json.get_ptr("children")) {
      children = factory.createList(*jchildren);
    }
  } else {
    children = factory.createList(json);
  }
  return detail::makeAllFastestRoute<RouterInfo>(std::move(children));
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
