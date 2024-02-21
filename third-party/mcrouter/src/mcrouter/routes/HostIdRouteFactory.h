/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/hash/Hash.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeHostIdRoute(
    std::vector<typename RouterInfo::RouteHandlePtr> rh,
    folly::StringPiece salt) {
  if (rh.empty()) {
    return createNullRoute<typename RouterInfo::RouteHandleIf>();
  }
  size_t hostIdHash = globals::hostid();
  if (!salt.empty()) {
    hostIdHash = folly::Hash()(hostIdHash, salt);
  }
  return std::move(rh[hostIdHash % rh.size()]);
}

} // namespace detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeHostIdRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  std::vector<typename RouterInfo::RouteHandlePtr> children;
  folly::StringPiece salt;
  if (json.isObject()) {
    if (auto jchildren = json.get_ptr("children")) {
      children = factory.createList(*jchildren);
    }
    if (auto jsalt = json.get_ptr("salt")) {
      checkLogic(jsalt->isString(), "HostIdRoute: salt is not a string");
      salt = jsalt->stringPiece();
    }
  } else {
    children = factory.createList(json);
  }

  return detail::makeHostIdRoute<RouterInfo>(std::move(children), salt);
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
