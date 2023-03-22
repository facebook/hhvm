/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/FuncGenerator.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/mc/msg.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeKeySplitRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "KeySplitRoute should be an object");
  checkLogic(json.count("destination"), "KeySplitRoute: no destination route");
  checkLogic(json.count("replicas"), "KeySplitRoute: no replicas specified");
  checkLogic(json.count("all_sync"), "KeySplitRoute: all_sync not specified");

  checkLogic(
      json["replicas"].isInt(), "KeySplitRoute: replicas is not an integer");
  checkLogic(
      json["all_sync"].isBool(), "KeySplitRoute: all_sync is not a boolean");

  auto isFirstHit = json.get_ptr("first_hit");
  if (isFirstHit) {
    checkLogic(
        isFirstHit->isBool(), "KeySplitRoute: first_hit is not a boolean");
  }

  size_t replicas = json["replicas"].getInt();
  bool all_sync = json["all_sync"].getBool();
  checkLogic(
      replicas >= KeySplitRoute<RouterInfo>::kMinReplicaCount,
      "KeySplitRoute: there should at least be 2 replicas");
  checkLogic(
      replicas <= KeySplitRoute<RouterInfo>::kMaxReplicaCount,
      "KeySplitRoute: there should no more than 1000 replicas");

  return makeRouteHandleWithInfo<RouterInfo, KeySplitRoute>(
      factory.create(json["destination"]),
      replicas,
      all_sync,
      isFirstHit ? isFirstHit->asBool() : false);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
