/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McRouteHandleProvider.h"

#include <mcrouter/routes/AllFastestRouteFactory.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

template MemcacheRouterInfo::RouteHandlePtr
makeAllFastestRoute<MemcacheRouterInfo>(
    RouteHandleFactory<MemcacheRouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
