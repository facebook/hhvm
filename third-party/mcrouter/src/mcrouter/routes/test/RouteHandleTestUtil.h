/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

using TestHandle = TestHandleImpl<McrouterRouteHandleIf>;

/**
 * Create mcrouter instance for test
 */
template <class RouterInfo = McrouterRouterInfo>
inline CarbonRouterInstance<RouterInfo>* getTestRouter() {
  McrouterOptions opts = defaultTestOptions();
  opts.config = "{ \"route\": \"NullRoute\" }";
  std::string name = "test_";
  name += RouterInfo::name;
  return CarbonRouterInstance<RouterInfo>::init(name, opts);
}

/**
 * Create recording ProxyRequestContext for fiber locals
 */
template <class RouterInfo = McrouterRouterInfo>
inline std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>>
getTestContext() {
  return ProxyRequestContextWithInfo<RouterInfo>::createRecording(
      *getTestRouter<RouterInfo>()->getProxy(0), nullptr);
}

/**
 * Set valid McrouterFiberContext in fiber locals
 */
template <class RouterInfo = McrouterRouterInfo>
inline void mockFiberContext() {
  std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>> ctx;
  folly::fibers::runInMainContext(
      [&ctx]() { ctx = getTestContext<RouterInfo>(); });
  fiber_local<RouterInfo>::setSharedCtx(std::move(ctx));
}

/**
 * Set valid McrouterFiberContext in fiber locals with record bucket data
 * context
 */
template <class RouterInfo = McrouterRouterInfo>
void setRecordBucketDataContext(
    std::vector<std::pair<std::string, std::string>>& pairs) {
  using KeyBucketPair = std::pair<std::string, std::string>;
  auto cb = [&pairs](
                std::string key,
                const uint64_t bucket,
                const std::string_view /*poolRecorded*/) {
    pairs.push_back({key, folly::to<std::string>(bucket)});
  };
  auto ctx = ProxyRequestContextWithInfo<RouterInfo>::createRecording(
      *getTestRouter<RouterInfo>()->getProxy(0),
      nullptr,
      nullptr,
      std::move(cb));
  fiber_local<RouterInfo>::setSharedCtx(std::move(ctx));
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
