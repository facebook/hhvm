/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Portability.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include "mcrouter/options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

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

class StatsApi;

/**
 * If linked, returns a reference to a StatsApi implementation allowing
 * custom stats handling.
 */
FOLLY_ATTR_WEAK StatsApi& gStatsApiHook();

/**
 * If linked, will be called once on router initialization with the intent
 * to initialize the custom StatsApi implementation.
 */
FOLLY_ATTR_WEAK void gStatsApiInitHook(const CarbonRouterInstanceBase& router);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
