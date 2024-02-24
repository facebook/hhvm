/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"
#include "mcrouter/routes/RoutingUtils.h"

namespace facebook::memcache::mcrouter {

struct KeyParseRouteSettings {
  // Number of parts to use for the routingKey
  size_t numRoutingParts;
  // The delimiter used to split the key
  char delimiter;
};

/**
 * This route handle parses the key and extracts a custom routingKey.
 * It forms the routingKey by splitting the request key using the configured
 * delimiter. It composes the routingKey from the first numRoutingParts after
 * splitting. If the key has less than numRoutingParts, it uses the full key.
 * The routingKey is put into fiber_local for subsequent route handles in the
 * tree to use.
 */
template <class RouterInfo>
class KeyParseRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  KeyParseRoute(RouteHandlePtr rh, KeyParseRouteSettings& settings)
      : rh_(std::move(rh)),
        numRoutingParts_(settings.numRoutingParts),
        delimiter_(settings.delimiter) {}

  std::string routeName() const {
    return fmt::format(
        "key_parse|delimiter={}|num_routing_parts={}",
        delimiter_,
        numRoutingParts_);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    auto routingKey = getPrefixWithDelimiters(getRoutingKey<Request>(req));
    return fiber_local<RouterInfo>::runWithLocals(
        [this, &req, &t, &routingKey]() {
          fiber_local<RouterInfo>::setCustomRoutingKey(std::move(routingKey));
          return t(*rh_, req);
        });
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    auto routingKey = getPrefixWithDelimiters(getRoutingKey<Request>(req));

    return fiber_local<RouterInfo>::runWithLocals([this, &req, &routingKey]() {
      fiber_local<RouterInfo>::setCustomRoutingKey(std::move(routingKey));
      return rh_->route(req);
    });
  }

 private:
  const RouteHandlePtr rh_;
  const size_t numRoutingParts_{0};
  const char delimiter_;

  FOLLY_ALWAYS_INLINE std::string getPrefixWithDelimiters(
      const folly::StringPiece key) const {
    size_t pos = 0;
    for (size_t i = 0; i < numRoutingParts_; ++i) {
      pos = key.find(delimiter_, pos);
      if (pos == std::string::npos) {
        break;
      }
      ++pos;
    }
    return key.subpiece(0, pos).str();
  }
};

KeyParseRouteSettings parseKeyParseRouteSettings(const folly::dynamic& json);

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeKeyParseRoute(
    typename RouterInfo::RouteHandlePtr rh,
    const folly::dynamic& json) {
  auto settings = parseKeyParseRouteSettings(json);
  return makeRouteHandleWithInfo<RouterInfo, KeyParseRoute>(
      std::move(rh), settings);
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeKeyParseRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "KeyParseRoute is not an object");
  checkLogic(json.count("target"), "KeyParseRoute: Missing target parameter");
  return makeKeyParseRoute<RouterInfo>(factory.create(json["target"]), json);
}

} // namespace facebook::memcache::mcrouter
