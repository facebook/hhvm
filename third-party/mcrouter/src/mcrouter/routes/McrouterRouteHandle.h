/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class Route>
using McrouterRouteHandle = MemcacheRouteHandle<Route>;

using McrouterRouteHandleIf = MemcacheRouteHandleIf;

using McrouterRouteHandlePtr = std::shared_ptr<McrouterRouteHandleIf>;

using McrouterRouterInfo = MemcacheRouterInfo;

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
