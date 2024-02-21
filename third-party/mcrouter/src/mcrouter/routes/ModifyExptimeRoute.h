/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/Range.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/MessageHelpers.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

enum class ModifyExptimeAction { Set, Min };

namespace detail {

inline ModifyExptimeAction stringToAction(folly::StringPiece s) {
  if (s == "set") {
    return ModifyExptimeAction::Set;
  } else if (s == "min") {
    return ModifyExptimeAction::Min;
  }
  checkLogic(false, "ModifyExptimeRoute: action should be 'set' or 'min'");
  return ModifyExptimeAction::Set;
}

inline const char* actionToString(ModifyExptimeAction action) {
  switch (action) {
    case ModifyExptimeAction::Set:
      return "set";
    case ModifyExptimeAction::Min:
      return "min";
  }
  assert(false);
  return "";
}

} // namespace detail

/**
 * Modifies exptime of a request.
 * Note that exptime is not modified for delete requests
 * If action == "set", applies a new expiration time.
 * If action == "min", applies a minimum of
 * (old expiration time, new expiration time).
 *
 * Note: 0 means infinite exptime.
 */
template <class RouteHandleIf, ModifyExptimeAction Action>
class ModifyExptimeRoute {
 public:
  std::string routeName() const {
    return folly::sformat(
        "modify-exptime|{}|exptime={}",
        detail::actionToString(Action),
        exptime_);
  }

  ModifyExptimeRoute(std::shared_ptr<RouteHandleIf> target, int32_t exptime)
      : target_(std::move(target)), exptime_(exptime) {
    assert(Action != ModifyExptimeAction::Min || exptime_ != 0);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(*target_, req);
  }

  template <class Request>
  typename std::enable_if<
      HasExptimeTrait<Request>::value &&
          carbon::OtherThan<Request, carbon::DeleteLike<>>::value &&
          Action == ModifyExptimeAction::Set,
      ReplyT<Request>>::type
  route(const Request& req) const {
    auto mutReq = req;
    mutReq.exptime_ref() = exptime_;
    return target_->route(mutReq);
  }

  template <class Request>
  typename std::enable_if<
      HasExptimeTrait<Request>::value &&
          carbon::OtherThan<Request, carbon::DeleteLike<>>::value &&
          Action == ModifyExptimeAction::Min,
      ReplyT<Request>>::type
  route(const Request& req) const {
    /* 0 means infinite exptime. Set minimum of request exptime, exptime. */
    if (*req.exptime_ref() == 0 || *req.exptime_ref() > exptime_) {
      auto mutReq = req;
      mutReq.exptime_ref() = exptime_;
      return target_->route(mutReq);
    }
    return target_->route(req);
  }

  template <class Request>
  typename std::enable_if<
      !HasExptimeTrait<Request>::value || carbon::DeleteLike<Request>::value,
      ReplyT<Request>>::type
  route(const Request& req) const {
    return target_->route(req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> target_;
  const int32_t exptime_;
};

template <class RouteHandleIf>
using ModifyExptimeRouteMin =
    ModifyExptimeRoute<RouteHandleIf, ModifyExptimeAction::Min>;

template <class RouteHandleIf>
using ModifyExptimeRouteSet =
    ModifyExptimeRoute<RouteHandleIf, ModifyExptimeAction::Set>;

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeModifyExptimeRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "ModifyExptimeRoute: should be an object");
  auto jtarget = json.get_ptr("target");
  checkLogic(jtarget, "ModifyExptimeRoute: no target");
  auto target = factory.create(*jtarget);
  auto jexptime = json.get_ptr("exptime");
  checkLogic(jexptime, "ModifyExptimeRoute: no exptime");
  checkLogic(jexptime->isInt(), "ModifyExptimeRoute: exptime is not an int");
  auto exptime = jexptime->getInt();

  ModifyExptimeAction action{ModifyExptimeAction::Set};
  if (auto jaction = json.get_ptr("action")) {
    checkLogic(
        jaction->isString(), "ModifyExptimeRoute: action is not a string");
    action = detail::stringToAction(jaction->getString());
  }

  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  if (action == ModifyExptimeAction::Min) {
    // 0 means infinite exptime
    if (exptime == 0) {
      return target;
    }

    return makeRouteHandle<RouteHandleIf, ModifyExptimeRouteMin>(
        std::move(target), exptime);
  } else if (action == ModifyExptimeAction::Set) {
    return makeRouteHandle<RouteHandleIf, ModifyExptimeRouteSet>(
        std::move(target), exptime);
  }

  // unknown action, ignore
  assert(false);
  return target;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
