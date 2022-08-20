/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McRouteHandleProvider.h"

#include <mcrouter/routes/HashRouteFactory.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

template MemcacheRouterInfo::RouteHandlePtr createHashRoute<MemcacheRouterInfo>(
    const folly::dynamic& json,
    std::vector<MemcacheRouterInfo::RouteHandlePtr> rh,
    size_t threadId);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
