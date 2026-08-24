/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <folly/Range.h>

#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/ShadowRouteIf.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ProxyBase;

/**
 * Interface to create additional route handles for McRouteHandleProvider.
 */
template <class RouterInfo>
class ExtraRouteHandleProviderIf {
 public:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

  virtual std::shared_ptr<RouteHandleIf> makeShadow(
      ProxyBase& proxy,
      std::shared_ptr<RouteHandleIf> destination,
      ShadowData<RouterInfo> data,
      folly::StringPiece shadowPolicy) = 0;

  virtual std::shared_ptr<RouteHandleIf> makeFailoverRoute(
      const folly::dynamic& json,
      std::vector<std::shared_ptr<RouteHandleIf>> children) = 0;

  virtual std::vector<std::shared_ptr<RouteHandleIf>> tryCreate(
      RouteHandleFactory<RouteHandleIf>& factory,
      folly::StringPiece type,
      const folly::dynamic& json) = 0;

  /**
   * Wraps the root of the route handle tree, after the config's entry point
   * has been turned into route handles.
   *
   * Lets a binary inject a route handle that must see every request no matter
   * how the config expresses its entry point (route vs routes, aliases,
   * prefix selectors). The wrapper sits above routing prefix selection, so it
   * is created exactly once per proxy.
   */
  virtual std::shared_ptr<RouteHandleIf> wrapRoot(
      std::shared_ptr<RouteHandleIf> root) {
    return root;
  }

  virtual ~ExtraRouteHandleProviderIf() {}
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
