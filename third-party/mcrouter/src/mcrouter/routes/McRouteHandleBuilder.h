/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <
    template <typename... Ignored>
    class R,
    typename... RArgs,
    typename... Args>
McrouterRouteHandlePtr makeMcrouterRouteHandle(Args&&... args) {
  return makeRouteHandle<McrouterRouteHandleIf, R, RArgs...>(
      std::forward<Args>(args)...);
}

template <
    template <typename... Ignored>
    class R,
    typename... RArgs,
    typename... Args>
McrouterRouteHandlePtr makeMcrouterRouteHandleWithInfo(Args&&... args) {
  return makeRouteHandleWithInfo<McrouterRouterInfo, R, RArgs...>(
      std::forward<Args>(args)...);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
