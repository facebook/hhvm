/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

using McrouterInstance = CarbonRouterInstance<MemcacheRouterInfo>;

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
