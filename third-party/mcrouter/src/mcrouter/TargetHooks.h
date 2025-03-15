/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Portability.h>
#include <folly/executors/IOThreadPoolExecutor.h>

namespace facebook {
namespace memcache {

class McrouterOptions;

namespace mcrouter {

class CarbonRouterInstanceBase;
class StatsApi;

/**
 * If linked, initializes and reports utilization to RIM.
 */
FOLLY_ATTR_WEAK bool gRIMReport(
    const std::vector<std::string>& tenancyHierarchyPath,
    const std::unordered_map<std::string, uint64_t>& resourceUsageMap);

/**
 * SR factory init hook
 */
FOLLY_ATTR_WEAK std::shared_ptr<void> gSRInitHook(
    std::shared_ptr<folly::IOThreadPoolExecutorBase>,
    const McrouterOptions&);

FOLLY_ATTR_WEAK void gAxonInitHook(
    CarbonRouterInstanceBase& router,
    std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool);

/**
 * If linked, will be called once on router initialization with the intent
 * to initialize the custom StatsApi implementation.
 */
FOLLY_ATTR_WEAK std::unique_ptr<StatsApi> gMakeStatsApiHook(
    const CarbonRouterInstanceBase& router);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
