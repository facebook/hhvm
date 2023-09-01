/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <fstream>
#include <string>
#include <type_traits>
#include <utility>

#include <folly/Conv.h>
#include <folly/Range.h>

#include "mcrouter/AsyncWriter.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/invalidation/McInvalidationDefs.h"
#include "mcrouter/lib/invalidation/McInvalidationKvPairs.h"
#include "mcrouter/lib/network/AccessPoint.h"
#include "mcrouter/lib/network/MessageHelpers.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

memcache::McDeleteRequest addDeleteRequestSource(
    const memcache::McDeleteRequest& req,
    memcache::McDeleteRequestSource source);

FOLLY_NOINLINE bool spoolAxonProxy(
    const ProxyBase& proxy,
    const memcache::McDeleteRequest& req,
    const std::shared_ptr<AxonContext>& axonCtx,
    uint64_t bucketId);

FOLLY_NOINLINE bool spoolAsynclog(
    ProxyBase* proxy,
    const memcache::McDeleteRequest& req,
    const std::shared_ptr<const AccessPoint>& host,
    bool keepRoutingPrefix,
    folly::StringPiece asynclogName);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
