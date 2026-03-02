/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/network/gen/MemcacheRouteHandleIf.h"

namespace facebook {
namespace memcache {

using TestRouteHandleIf = MemcacheRouteHandleIf;

template <class Route>
using TestRouteHandle = MemcacheRouteHandle<Route>;

} // namespace memcache
} // namespace facebook
