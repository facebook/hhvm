/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <thrift/lib/cpp/server/TServerObserver.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

extern thread_local size_t tlsWorkerThreadId;

template <class RouterInfo>
class ThriftObserver final : public apache::thrift::server::TServerObserver {
 public:
  ThriftObserver(
      CarbonRouterInstance<RouterInfo>& router,
      std::shared_ptr<std::atomic<bool>> startedShutdown)
      : TServerObserver(0),
        router_(router),
        startedShutdown_(startedShutdown) {}

  void connAccepted(const wangle::TransportInfo& /* info */) override final {
    if (!startedShutdown_->load()) {
      auto proxy = router_.getProxy(tlsWorkerThreadId);
      proxy->stats().increment(num_client_connections_stat);
    }
  }

  void connClosed() override final {
    if (!startedShutdown_->load()) {
      auto proxy = router_.getProxy(tlsWorkerThreadId);
      proxy->stats().decrement(num_client_connections_stat);
    }
  }

 private:
  CarbonRouterInstance<RouterInfo>& router_;
  std::shared_ptr<std::atomic<bool>> startedShutdown_;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
