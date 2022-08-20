/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McExtraRouteHandleProvider.h"

#include <folly/Range.h>

#include "mcrouter/ProxyBase.h"
#include "mcrouter/routes/DefaultShadowPolicy.h"
#include "mcrouter/routes/FailoverRoute.h"
#include "mcrouter/routes/ShadowRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
McExtraRouteHandleProvider<RouterInfo>::makeShadow(
    ProxyBase& proxy,
    std::shared_ptr<typename RouterInfo::RouteHandleIf> destination,
    ShadowData<RouterInfo> data,
    folly::StringPiece shadowPolicy) {
  if (shadowPolicy == "default") {
    return makeShadowRouteDefault<RouterInfo>(
        std::move(destination),
        std::move(data),
        DefaultShadowPolicy(proxy.router()));
  } else {
    throw std::logic_error("Invalid shadow policy: " + shadowPolicy.str());
  }
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
McExtraRouteHandleProvider<RouterInfo>::makeFailoverRoute(
    const folly::dynamic& json,
    std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> children) {
  return makeFailoverRouteDefault<RouterInfo, FailoverRoute>(
      json, std::move(children));
}

template <class RouterInfo>
std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
McExtraRouteHandleProvider<RouterInfo>::tryCreate(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>&,
    folly::StringPiece /* type */,
    const folly::dynamic& /* json */) {
  return {};
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
