/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WarmUpRoute.h"

#include <folly/json/dynamic.h>

#include "mcrouter/config.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

McrouterRouteHandlePtr makeWarmUpRoute(
    McrouterRouteHandlePtr warm,
    McrouterRouteHandlePtr cold,
    folly::Optional<uint32_t> exptime) {
  return makeMcrouterRouteHandle<WarmUpRoute>(
      std::move(warm), std::move(cold), std::move(exptime));
}

McrouterRouteHandlePtr makeWarmUpRoute(
    RouteHandleFactory<McrouterRouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "WarmUpRoute should be object");
  checkLogic(json.count("cold"), "WarmUpRoute: no cold route");
  checkLogic(json.count("warm"), "WarmUpRoute: no warm route");
  bool enableMetaget = isMetagetAvailable();
  if (auto jenableMetaget = json.get_ptr("enable_metaget")) {
    checkLogic(
        jenableMetaget->isBool(),
        "WarmUpRoute: enable_metaget is not a boolean");
    enableMetaget = jenableMetaget->getBool();
  }
  folly::Optional<uint32_t> exptime;
  if (auto jexptime = json.get_ptr("exptime")) {
    checkLogic(jexptime->isInt(), "WarmUpRoute: exptime is not an integer");
    exptime = jexptime->getInt();
  } else if (!enableMetaget) {
    exptime = 0;
  }

  return makeWarmUpRoute(
      factory.create(json["warm"]),
      factory.create(json["cold"]),
      std::move(exptime));
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
