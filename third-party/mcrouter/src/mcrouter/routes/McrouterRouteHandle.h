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

template <typename Request>
struct MemcacheUpdateLike {
  static const bool value =
      carbon::ListContains<typename MemcacheRouterInfo::AllRequests, Request>::
          value &&
      carbon::UpdateLike<Request>::value;
};

template <typename Request>
struct OtherThanMemcacheGetUpdateLike {
  static const bool value =
      !carbon::ListContains<typename MemcacheRouterInfo::AllRequests, Request>::
          value ||
      (!carbon::GetLike<Request>::value && !carbon::UpdateLike<Request>::value);
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
