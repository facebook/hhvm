/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McRouteHandleProvider.h"

#include <mcrouter/routes/FailoverRoute.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

template MemcacheRouterInfo::RouteHandlePtr
makeFailoverRouteWithFailoverErrorSettings<
    MemcacheRouterInfo,
    FailoverRoute,
    FailoverErrorsSettings>(
    const folly::dynamic& json,
    std::vector<MemcacheRouterInfo::RouteHandlePtr> children,
    FailoverErrorsSettings failoverErrors,
    const folly::dynamic* jFailoverPolicy);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
