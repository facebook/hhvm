/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/routes/ExtraRouteHandleProviderIf.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ProxyBase;

/**
 * Creates additional route handles for McRouteHandleProvider.
 */
template <class RouterInfo>
class McExtraRouteHandleProvider
    : public ExtraRouteHandleProviderIf<RouterInfo> {
 public:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

  std::shared_ptr<RouteHandleIf> makeShadow(
      ProxyBase& proxy,
      std::shared_ptr<RouteHandleIf> destination,
      ShadowData<RouterInfo> data,
      folly::StringPiece shadowPolicy) override;

  std::shared_ptr<RouteHandleIf> makeFailoverRoute(
      const folly::dynamic& json,
      std::vector<std::shared_ptr<RouteHandleIf>> children) override;

  std::vector<std::shared_ptr<RouteHandleIf>> tryCreate(
      RouteHandleFactory<RouteHandleIf>& factory,
      folly::StringPiece type,
      const folly::dynamic& json) override;

  ~McExtraRouteHandleProvider() override {}
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "McExtraRouteHandleProvider-inl.h"
