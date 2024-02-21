/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/json/dynamic.h>

#include "mcrouter/lib/routes/NullRoute.h"
#include "mcrouter/routes/BlackholingPolicyVisitor.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Blackholes requests based on policies
 */
template <class RouterInfo>
class BlackholeRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  /**
   * Create blackhole route
   *
   * Sample json format:
   * {
   *  "policies": {
   *    "fieldName": [ // we use an 'or' of policies for the same field
   *      {
   *        "op": "equals", // equals or not_equals
   *        "value": 1234   // int, double, bool or string
   *      }
   *    ]
   *  },
   *  "default": "ErrorRoute"
   *  "blackhole_child": "NullRoute"
   * }
   *
   * @param defaultRh       The default route handle
   * @param blackholeRh     The route handle used to blackhole, NullRoute by
   *                        default
   * @param jsonMap         Map that has fields and corresponding policies
   */
  BlackholeRoute(
      RouteHandlePtr defaultRh,
      RouteHandlePtr blackholeRh,
      const folly::dynamic& jsonMap)
      : defaultRh_(std::move(defaultRh)),
        blackholeRh_(std::move(blackholeRh)),
        jsonMap_(jsonMap) {
    assert(defaultRh_);
    assert(blackholeRh_);
  }

  static std::string routeName() {
    return "blackhole";
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    if (shouldBlackhole(req, jsonMap_)) {
      return blackholeRh_->route(req);
    }

    return defaultRh_->route(req);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (shouldBlackhole(req, jsonMap_)) {
      return t(*blackholeRh_, req);
    }
    return t(*defaultRh_, req);
  }

 private:
  const RouteHandlePtr defaultRh_;
  const RouteHandlePtr blackholeRh_;
  const folly::dynamic jsonMap_;
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeBlackholeRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "BlackholeRoute-inl.h"
