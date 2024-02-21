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
#include "mcrouter/lib/routes/AllMajorityRoute.h"
#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createAllMajorityRoute(
    std::vector<typename RouterInfo::RouteHandlePtr> rh) {
  if (rh.empty()) {
    return createNullRoute<typename RouterInfo::RouteHandleIf>();
  }

  if (rh.size() == 1) {
    return std::move(rh[0]);
  }

  return makeRouteHandle<typename RouterInfo::RouteHandleIf, AllMajorityRoute>(
      std::move(rh));
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeAllMajorityRoute(
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
  return createAllMajorityRoute<RouterInfo>(std::move(children));
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
