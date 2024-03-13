/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/Ch3HashFunc.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/LowerBoundPrefixMap.h"
#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"
#include "mcrouter/routes/RoutingUtils.h"

namespace facebook::memcache::mcrouter {

struct HashStopAllowListRouteSettings {
  std::vector<std::pair<std::string, bool>> prefixes;
};

/**
 * This route handle allow lists uses of the mcrouter HashStop.
 * Keys which have a hashstop and are not configured in the allow
 * list will return LOCAL_ERROR.
 */
template <class RouterInfo>
class HashStopAllowListRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  HashStopAllowListRoute(
      RouteHandlePtr rh,
      HashStopAllowListRouteSettings& settings)
      : rh_(std::move(rh)), prefixMap_(settings.prefixes) {}

  std::string routeName() const {
    return fmt::format(
        "HashStopAllowListRoute|num_prefixes={}", prefixMap_.size());
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (FOLLY_LIKELY(!req.key_ref()->hasHashStop()) ||
        allowedHashStop(req.key_ref()->fullKey())) {
      return t(*rh_, req);
    }
    return false;
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    if (FOLLY_LIKELY(!req.key_ref()->hasHashStop()) ||
        allowedHashStop(req.key_ref()->fullKey())) {
      return rh_->route(req);
    }
    return createReply<Request>(
        ErrorReply, carbon::Result::LOCAL_ERROR, "Hash stop not permited");
  }

 private:
  const RouteHandlePtr rh_;
  const memcache::LowerBoundPrefixMap<bool> prefixMap_;

  FOLLY_ALWAYS_INLINE bool allowedHashStop(const folly::StringPiece key) const {
    if (prefixMap_.empty()) {
      return false;
    }
    auto it = prefixMap_.findPrefix(key);
    return it != prefixMap_.end();
  }
};

HashStopAllowListRouteSettings parseHashStopAllowListRouteSettings(
    const folly::dynamic& json);

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createHashStopAllowListRoute(
    typename RouterInfo::RouteHandlePtr rh,
    const folly::dynamic& json) {
  auto settings = parseHashStopAllowListRouteSettings(json);
  return makeRouteHandleWithInfo<RouterInfo, HashStopAllowListRoute>(
      std::move(rh), settings);
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeHashStopAllowListRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "HashStopAllowListRoute is not an object");
  checkLogic(
      json.count("target"), "HashStopAllowListRoute: Missing target parameter");
  return createHashStopAllowListRoute<RouterInfo>(
      factory.create(json["target"]), json);
}

} // namespace facebook::memcache::mcrouter
