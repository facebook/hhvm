/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McRouteHandleProvider.h"

#include "mcrouter/routes/CarbonLookasideRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * This implementation is only for test purposes. Typically the users of
 * CarbonLookaside will be services other than memcache.
 */
class MemcacheCarbonLookasideHelper {
 public:
  explicit MemcacheCarbonLookasideHelper(
      const folly::dynamic* /* jsonConfig */) {}

  static std::string name() {
    return "MemcacheCarbonLookasideHelper";
  }

  template <typename Request>
  bool cacheCandidate(const Request& /* unused */) const {
    if (HasKeyTrait<Request>::value) {
      return true;
    }
    return false;
  }

  template <typename Request>
  std::string buildKey(const Request& req) const {
    if (HasKeyTrait<Request>::value) {
      return req.key_ref()->fullKey().str();
    }
    return std::string();
  }

  template <typename Reply>
  bool shouldCacheReply(const Reply& /* unused */) const {
    return true;
  }

  template <typename Reply>
  void postProcessCachedReply(Reply& /* reply */) const {}
};

template MemcacheRouterInfo::RouteHandlePtr
createCarbonLookasideRoute<MemcacheRouterInfo, MemcacheCarbonLookasideHelper>(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
