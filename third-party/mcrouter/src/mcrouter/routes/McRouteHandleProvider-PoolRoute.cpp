/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McRouteHandleProvider.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template std::tuple<
    std::vector<MemcacheRouterInfo::RouteHandlePtr>,
    std::optional<folly::dynamic>>
McRouteHandleProvider<MemcacheRouterInfo>::makePool(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const PoolFactory::PoolJson& json);

template MemcacheRouterInfo::RouteHandlePtr
McRouteHandleProvider<MemcacheRouterInfo>::makePoolRoute(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
