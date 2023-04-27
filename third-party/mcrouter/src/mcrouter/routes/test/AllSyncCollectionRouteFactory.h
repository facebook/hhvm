/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/test/AllSyncCollectionRoute.h"
#include "mcrouter/routes/CollectionRouteFactory.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createAllSyncCollectionRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  return createCollectionRoute<RouterInfo, AllSyncCollectionRoute>(
      factory, json);
}

} // end namespace mcrouter

} // end namespace memcache
} // end namespace facebook
