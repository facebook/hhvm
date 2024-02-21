/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StagingRoute.h"

#include <folly/json/dynamic.h>

#include "mcrouter/config.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

McrouterRouteHandlePtr makeStagingRoute(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "StagingRoute should be an object");

  auto jWarm = json.get_ptr("warm");
  checkLogic(jWarm != nullptr, "StagingRoute: warm route property is missing");

  auto jStaging = json.get_ptr("staging");
  checkLogic(
      jStaging != nullptr, "StagingRoute: staging route property is missing");

  return std::make_shared<MemcacheRouteHandle<StagingRoute>>(
      factory.create(*jWarm), factory.create(*jStaging));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
