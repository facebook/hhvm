/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/network/MessageHelpers.h"
#include "mcrouter/routes/CarbonLookasideRoute.h"

namespace folly {
struct dynamic;
} // namespace folly

namespace hellogoodbye {

class HelloGoodbyeCarbonLookasideHelper {
 public:
  HelloGoodbyeCarbonLookasideHelper(const folly::dynamic* /* jsonConfig */) {}

  static std::string name() {
    return "HelloGoodbyeCarbonLookasideHelper";
  }

  template <typename Request>
  bool cacheCandidate(const Request& /* unused */) {
    if (facebook::memcache::HasKeyTrait<Request>::value) {
      return true;
    }
    return false;
  }

  template <typename Request>
  std::string buildKey(const Request& req) {
    if (facebook::memcache::HasKeyTrait<Request>::value) {
      return req.key_ref()->fullKey().str();
    }
    return std::string();
  }

  template <typename Reply>
  bool shouldCacheReply(const Reply& /* reply */) const {
    return true;
  }

  template <typename Reply>
  void postProcessCachedReply(Reply& /* reply */) const {}
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeCarbonLookasideRoute(
    facebook::memcache::RouteHandleFactory<typename RouterInfo::RouteHandleIf>&
        factory,
    const folly::dynamic& json) {
  return facebook::memcache::mcrouter::
      createCarbonLookasideRoute<RouterInfo, HelloGoodbyeCarbonLookasideHelper>(
          factory, json);
}

} // namespace hellogoodbye
