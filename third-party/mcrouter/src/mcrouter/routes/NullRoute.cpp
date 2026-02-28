/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/json/dynamic.h>

#include "mcrouter/lib/routes/NullRoute.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

McrouterRouteHandlePtr makeNullRoute(
    RouteHandleFactory<McrouterRouteHandleIf>& factory,
    const folly::dynamic& json) {
  return makeNullRoute<McrouterRouteHandleIf>(factory, json);
}

McrouterRouteHandlePtr makeNullRoute() {
  return createNullRoute<McrouterRouteHandleIf>();
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
