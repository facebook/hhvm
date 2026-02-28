/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <folly/io/async/HHWheelTimer.h>
#include <optional>
#include <proxygen/lib/http/coro/client/HTTPCoroConnector.h>
#include <proxygen/lib/http/coro/client/HTTPCoroSessionPool.h>

namespace proxygen::coro {

/**
 * An HTTPCoroSessionPool that invokes a user-provided callback on a
 * configurable interval. This is useful for periodically reloading client
 * certificates to prevent expiration issues in long-lived tasks.
 *
 * This class derives from HTTPCoroSessionPool, inheriting all its constructors.
 * It can be used anywhere an HTTPCoroSessionPool is expected, including as a
 * proxyPool parameter.
 *
 * The user is responsible for providing a callback that performs the desired
 * action (e.g., reloading certificates and calling setConnParams()).
 *
 * Example usage:
 *   auto pool = std::make_shared<CertReloadSessionPool>(
 *       evb, "server.example.com", 443, poolParams, connParams, sessionParams);
 *
 *   pool->setTimerCallback(
 *       [](HTTPCoroSessionPool& p) {
 *         TLSParams tlsParams;
 *         tlsParams.clientCertPath = "/path/to/cert";
 *         tlsParams.clientKeyPath = "/path/to/key";
 *         auto connParams = HTTPCoroConnector::ConnectionParams{};
 *         connParams.fizzContextAndVerifier.fizzContext =
 *             HTTPCoroConnector::makeFizzClientContext(tlsParams);
 *         p.setConnParams(connParams);
 *       },
 *       std::chrono::milliseconds(600000)); // 10 minutes
 */
class CertReloadSessionPool : public HTTPCoroSessionPool {
 public:
  using HTTPCoroSessionPool::HTTPCoroSessionPool;

  ~CertReloadSessionPool() override;

  CertReloadSessionPool(const CertReloadSessionPool&) = delete;
  CertReloadSessionPool& operator=(const CertReloadSessionPool&) = delete;
  CertReloadSessionPool(CertReloadSessionPool&&) = delete;
  CertReloadSessionPool& operator=(CertReloadSessionPool&&) = delete;

  void setTimerCallback(std::function<void(HTTPCoroSessionPool&)> cb,
                        std::chrono::milliseconds interval);

 private:
  class ReloadTimerCallback : public folly::HHWheelTimer::Callback {
   public:
    ReloadTimerCallback(CertReloadSessionPool& pool,
                        std::function<void(HTTPCoroSessionPool&)> cb,
                        std::chrono::milliseconds interval)
        : pool_(pool), cb_(std::move(cb)), interval_(interval) {
      schedule();
    }

    ~ReloadTimerCallback() override {
      cancelTimeout();
    }

    ReloadTimerCallback(const ReloadTimerCallback&) = delete;
    ReloadTimerCallback& operator=(const ReloadTimerCallback&) = delete;
    ReloadTimerCallback(ReloadTimerCallback&&) = delete;
    ReloadTimerCallback& operator=(ReloadTimerCallback&&) = delete;

    void timeoutExpired() noexcept override {
      cb_(pool_);
      schedule();
    }

    void callbackCanceled() noexcept override {
    }

   private:
    void schedule() {
      pool_.getEventBase()->timer().scheduleTimeout(this, interval_);
    }

    CertReloadSessionPool& pool_;
    std::function<void(HTTPCoroSessionPool&)> cb_;
    std::chrono::milliseconds interval_;
  };

  std::optional<ReloadTimerCallback> reloadTimer_;
};

} // namespace proxygen::coro
