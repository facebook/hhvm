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
 * SR factory init hook
 */
FOLLY_ATTR_WEAK std::shared_ptr<void> gSRInitHook(
    std::shared_ptr<folly::IOThreadPoolExecutorBase>,
    const std::string& /* threadPrefix */,
    const McrouterOptions&);

FOLLY_ATTR_WEAK void gAxonInitHook(
    CarbonRouterInstanceBase& router,
    std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool,
    const std::string& threadPrefix);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
