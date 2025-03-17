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

constexpr std::string_view kBroadcast = "DistributionRoute";
constexpr std::string_view kDistributionTargetMarkerForLog = "dl_distribution";

memcache::McDeleteRequest addDeleteRequestSource(
    const memcache::McDeleteRequest& req,
    memcache::McDeleteRequestSource source);

/**
 * Distribute write requests to Distribution layer.
 * `targetRegion` value for cross-region directed writes
 * will be the name of the region. For broadcasts it will be a
 * `kBroadcast` value.
 * In case of broadcast writes, `sourceRegion` value is used in WH filters to
 * determine the region that will be excluded from consuming this write.
 *
 * @param req Request to distribute
 * @param axonCtx Axon context
 * @param bucketId Bucket id
 * @param targetRegion Target region for the distribution
 * @param sourceRegion Source (client) region of the distribution
 * @param message Optional message to pass down with the record
 * @param secureWrites enable client identity check for writes
 */
FOLLY_NOINLINE bool distributeWriteRequest(
    const memcache::McSetRequest& req,
    const std::shared_ptr<AxonContext>& axonCtx,
    uint64_t bucketId,
    const std::string& targetRegion,
    const std::string& sourceRegion,
    std::optional<std::string> message = std::nullopt,
    bool secureWrites = false);

/**
 * Distribute delete requests to Distribution layer.
 * For broadcasts the `targetRegion` value will be the
 * `kBroadcast` value, for cross-region directed deletes it will
 * be the name of the region, for same-region failed async invalidations
 * it will be the name of the local region.
 * `sourceRegion` value is used in WH filters to determine the region that will
 * be excluded from consuming this delete.
 *
 * @param req Request to distribute
 * @param axonCtx Axon context
 * @param bucketId Bucket id
 * @param type Type of the delete distribution (async/distribution)
 * @param targetRegion Target region for the distribution
 * @param sourceRegion Source (client) region of the distribution
 * @param message Optional message to pass down with the record
 */
FOLLY_NOINLINE bool distributeDeleteRequest(
    const memcache::McDeleteRequest& req,
    const std::shared_ptr<AxonContext>& axonCtx,
    uint64_t bucketId,
    invalidation::DistributionType type,
    const std::string& targetRegion = "",
    const std::string& sourceRegion = "",
    std::optional<std::string> message = std::nullopt);

FOLLY_NOINLINE bool spoolAsynclog(
    ProxyBase* proxy,
    const memcache::McDeleteRequest& req,
    const std::shared_ptr<const AccessPoint>& host,
    bool keepRoutingPrefix,
    folly::StringPiece asynclogName);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
