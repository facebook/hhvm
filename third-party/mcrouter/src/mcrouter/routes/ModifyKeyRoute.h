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
#include <folly/Optional.h>
#include <folly/json/dynamic.h>

#include "mcrouter/RoutingPrefix.h"
#include "mcrouter/lib/McKey.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/TypeList.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Modifies key of current request.
 *  set_routing_prefix if present, routing prefix of a key will be set to this
 *                     value
 *  ensure_key_prefix if key doesn't start with this value, it will be prepended
 *                    to the key
 *  replace_key_prefix Replace this prefix if it exists, and append
 *                     ensure_key_prefix
 *  set_key_suffix If present, value will be appended to this key.
 *
 * Example:
 *  ModifyKeyRoute
 *    set_routing_prefix = "/a/b/"
 *    ensure_key_prefix = "foo"
 * "/a/b/a" => "/a/b/fooa"
 * "foo" => "/a/b/foo"
 * "/b/c/o" => "/a/b/fooo"
 */
template <class RouteHandleIf>
class ModifyKeyRoute {
 public:
  static std::string routeName() {
    return "modify-key";
  }

  ModifyKeyRoute(
      std::shared_ptr<RouteHandleIf> target,
      folly::Optional<std::string> routingPrefix,
      std::string keyPrefix,
      bool modifyInplace,
      folly::Optional<std::string> keyReplace,
      std::string keySuffix)
      : target_(std::move(target)),
        routingPrefix_(std::move(routingPrefix)),
        keyPrefix_(std::move(keyPrefix)),
        modifyInplace_(modifyInplace),
        keyReplace_(std::move(keyReplace)),
        keySuffix_(std::move(keySuffix)) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    auto cloneReq = req;
    auto key = getModifiedKey(*req.key_ref());
    if (key) {
      cloneReq.key_ref() = key.value();
    }
    return t(*target_, cloneReq);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    const auto key = getModifiedKey(*req.key_ref());
    if (key) {
      return routeReqWithKey(req, key.value());
    }
    return target_->route(req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> target_;
  const folly::Optional<std::string> routingPrefix_;
  const std::string keyPrefix_;
  const bool modifyInplace_;
  const folly::Optional<std::string> keyReplace_;
  const std::string keySuffix_;

  template <class StringLike>
  folly::Optional<std::string> getModifiedKey(
      const carbon::Keys<StringLike>& reqKey) const {
    folly::StringPiece rp = routingPrefix_.hasValue() ? routingPrefix_.value()
                                                      : reqKey.routingPrefix();

    if (keyReplace_.hasValue() &&
        reqKey.keyWithoutRoute().startsWith(keyReplace_.value())) {
      auto keyWithoutRoute = reqKey.keyWithoutRoute();
      keyWithoutRoute.advance(keyReplace_.value().size());
      return folly::to<std::string>(
          rp, keyPrefix_, keyWithoutRoute, keySuffix_);
    } else if (!reqKey.keyWithoutRoute().startsWith(keyPrefix_)) {
      auto keyWithoutRoute = reqKey.keyWithoutRoute();
      if (modifyInplace_ && keyWithoutRoute.size() >= keyPrefix_.size()) {
        keyWithoutRoute.advance(keyPrefix_.size());
      }
      return folly::to<std::string>(
          rp, keyPrefix_, keyWithoutRoute, keySuffix_);
    } else if (routingPrefix_.hasValue() && rp != reqKey.routingPrefix()) {
      return folly::to<std::string>(rp, reqKey.keyWithoutRoute(), keySuffix_);
    } else if (!keySuffix_.empty()) {
      return folly::to<std::string>(reqKey.fullKey(), keySuffix_);
    }
    return folly::none;
  }

  template <class Request>
  ReplyT<Request> routeReqWithKey(const Request& req, folly::StringPiece key)
      const {
    constexpr bool kIsMemcacheRequest =
        ListContains<McRequestList, Request>::value;
    const auto err = isKeyValid<kIsMemcacheRequest>(key);
    if (err != mc_req_err_valid) {
      return createReply<Request>(
          ErrorReply,
          "ModifyKeyRoute: invalid key: " +
              std::string(mc_req_err_to_string(err)));
    }
    auto cloneReq = req;
    cloneReq.key_ref() = key;
    return target_->route(cloneReq);
  }
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeModifyKeyRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  auto jtarget = json.get_ptr("target");
  checkLogic(jtarget, "ModifyKeyRoute: no target");

  folly::Optional<std::string> routingPrefix;
  if (auto jroutingPrefix = json.get_ptr("set_routing_prefix")) {
    auto rp = jroutingPrefix->stringPiece();
    if (rp.empty()) {
      routingPrefix = "";
    } else {
      try {
        routingPrefix = RoutingPrefix(rp).str();
      } catch (const std::exception& e) {
        throw std::logic_error(
            "ModifyKeyRoute: set_routing_prefix: " + std::string(e.what()));
      }
    }
  }
  std::string keyPrefix;
  if (auto jkeyPrefix = json.get_ptr("ensure_key_prefix")) {
    keyPrefix = jkeyPrefix->getString();
    auto err = isKeyValid<true /* DoSpaceAndCtrlCheck */>(keyPrefix);
    checkLogic(
        keyPrefix.empty() || err == mc_req_err_valid,
        "ModifyKeyRoute: invalid key prefix '{}', {}",
        keyPrefix,
        mc_req_err_to_string(err));
  }

  folly::Optional<std::string> keyReplace;
  if (auto jkeyReplace = json.get_ptr("replace_key_prefix")) {
    keyReplace = jkeyReplace->getString();
    auto err = isKeyValid<true /* DoSpaceAndCtrlCheck */>(keyReplace.value());
    checkLogic(
        keyReplace.value().empty() || err == mc_req_err_valid,
        "ModifyKeyRoute: invalid key prefix '{}', {}",
        keyReplace.value(),
        mc_req_err_to_string(err));
  }

  std::string keySuffix;
  if (auto jkeySuffix = json.get_ptr("set_key_suffix")) {
    keySuffix = jkeySuffix->getString();
    auto err = isKeyValid<true /* DoSpaceAndCtrlCheck */>(keySuffix);
    checkLogic(
        keySuffix.empty() || err == mc_req_err_valid,
        "ModifyKeyRoute: invalid key suffix '{}', {}",
        keySuffix,
        mc_req_err_to_string(err));
  }

  bool modifyInplace = false;
  if (auto joverwrite = json.get_ptr("modify_inplace")) {
    checkLogic(
        !keyReplace.hasValue(),
        "replace_key_prefix and modify_inplace cannot be used together");
    checkLogic(
        joverwrite->isBool(), "ModifyKeyRoute: modify_inplace is not a bool");
    modifyInplace = joverwrite->asBool();
  }
  return makeRouteHandle<typename RouterInfo::RouteHandleIf, ModifyKeyRoute>(
      factory.create(*jtarget),
      std::move(routingPrefix),
      std::move(keyPrefix),
      modifyInplace,
      std::move(keyReplace),
      std::move(keySuffix));
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
