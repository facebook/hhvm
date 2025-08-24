/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/client/HTTPCoroSessionPool.h"
#include <fizz/client/SynchronizedLruPskCache.h>
#include <folly/container/EvictingCacheMap.h>
#include <folly/logging/xlog.h>

namespace proxygen::coro {

constexpr uint32_t kDefaultMaxConnectionPools = 10;
constexpr uint32_t kPskCacheSize = 10000;

/**
 * This class holds an evicting cache map of session pools.  It can be used
 * by a client that will be used to make connections to a variety of
 * destinations over time.
 */
class HTTPClientConnectionCache : public HTTPSessionFactory {
 public:
  struct ProxyParams {
    std::string server;
    uint16_t port;
    bool useConnect{false};
    HTTPCoroSessionPool::PoolParams poolParams;
    HTTPCoroConnector::ConnectionParams connParams;
    HTTPCoroConnector::SessionParams sessionParams;
  };

  explicit HTTPClientConnectionCache(
      folly::EventBase& evb,
      folly::Optional<ProxyParams> proxyParams = folly::none,
      const uint32_t maxConnectionPools = kDefaultMaxConnectionPools)
      : eventBase_(evb),
        proxyPool_(proxyParams ? std::make_shared<HTTPCoroSessionPool>(
                                     &eventBase_,
                                     proxyParams->server,
                                     proxyParams->port,
                                     proxyParams->poolParams,
                                     getProxyConnParams(
                                         std::move(proxyParams->connParams)),
                                     proxyParams->sessionParams,
                                     /*allowNameLookup=*/true)
                               : nullptr),
        useConnectForProxy_(proxyParams && proxyParams->useConnect),
        pools_(maxConnectionPools) {
  }
  ~HTTPClientConnectionCache() override;

  [[nodiscard]] folly::EventBase* getEventBase() const {
    return &eventBase_;
  }

  [[nodiscard]] bool requiresAbsoluteURLs() const override {
    return usingProxy() && !useConnectForProxy_;
  }

  void setPoolParams(HTTPCoroSessionPool::PoolParams poolParams) {
    poolParams_ = poolParams;
  }

  void setConnParams(HTTPCoroConnector::ConnectionParams connParams) {
    XCHECK(connParams.sslContext ||
           connParams.fizzContextAndVerifier.fizzContext)
        << "Must set secure ConnectionParams";
    XCHECK(!usingProxy() || useConnectForProxy_)
        << "Cannot set connParams with non CONNECT proxy";
    connParams_ = std::move(connParams);
  }

  void setSessionParams(HTTPCoroConnector::SessionParams sessParams) {
    sessionParams_ = std::move(sessParams);
  }

  folly::coro::Task<GetSessionResult> getSessionWithReservation(
      std::string url,
      std::chrono::milliseconds timeout,
      const HTTPCoroConnector::ConnectionParams* connParams = nullptr);

  folly::coro::Task<GetSessionResult> getSessionWithReservation(
      std::string host,
      uint16_t port,
      bool isSecure,
      std::chrono::milliseconds timeout,
      folly::Optional<std::string> serverAddress = folly::none) override;

  folly::coro::Task<GetSessionResult> getSessionWithReservation(
      std::string host,
      uint16_t port,
      bool isSecure,
      std::chrono::milliseconds timeout,
      const HTTPCoroConnector::ConnectionParams* connParams,
      folly::Optional<std::string> serverAddress = folly::none);

  size_t getNumPools() const {
    return pools_.size();
  }

  [[nodiscard]] size_t getMaxNumPools() const {
    return pools_.getMaxSize();
  }

  void drain();

  void reapEmptyPools(size_t maxPoolsToTraverse);

  void reapAllEmptyPools() {
    reapEmptyPools(pools_.size());
  }

 protected:
  // Checking if a pool exists does not cause a promotion in the pools LRU
  bool poolExists(
      folly::StringPiece address,
      uint16_t port,
      bool isSecure,
      const HTTPCoroConnector::ConnectionParams* connParams = nullptr);

  /**
   * This function either creates a pool or returns an existing one if present.
   * Pools are indexed by their (address, port, isSecure) tuple; however if
   * multiple HTTPCoroSessionPools objects to the same (address, port, isSecure)
   * tuple are needed, you may pass in a ConnectionParams ptr that extends the
   * tuple index to become (address, port, isSecure, hash(&sslCtx,
   * &fizzContextAndVerifier))
   */
  HTTPCoroSessionPool& getPool(
      folly::StringPiece host,
      folly::StringPiece address,
      uint16_t port,
      bool isSecure,
      const HTTPCoroConnector::ConnectionParams* connParams = nullptr);

 private:
  HTTPCoroConnector::ConnectionParams getProxyConnParams(
      HTTPCoroConnector::ConnectionParams connParams) {
    if (connParams.fizzContextAndVerifier.fizzContext &&
        !connParams.fizzContextAndVerifier.fizzContext->getPskCache()) {
      auto newFizzCtx = std::make_shared<fizz::client::FizzClientContext>(
          *connParams.fizzContextAndVerifier.fizzContext);
      newFizzCtx->setPskCache(pskCache_);
      connParams.fizzContextAndVerifier.fizzContext = std::move(newFizzCtx);
    }
    return connParams;
  }

  [[nodiscard]] bool usingProxy() const {
    return bool(proxyPool_);
  }

  folly::EventBase& eventBase_;
  std::shared_ptr<fizz::client::SynchronizedLruPskCache> pskCache_{
      std::make_shared<fizz::client::SynchronizedLruPskCache>(kPskCacheSize)};
  folly::Optional<HTTPCoroSessionPool::PoolParams> poolParams_;
  folly::Optional<HTTPCoroConnector::ConnectionParams> connParams_;
  folly::Optional<HTTPCoroConnector::SessionParams> sessionParams_;
  std::shared_ptr<HTTPCoroSessionPool> proxyPool_;
  folly::CancellationSource cancellationSource_;
  bool useConnectForProxy_{false};
  folly::EvictingCacheMap<std::string, std::unique_ptr<HTTPCoroSessionPool>>
      pools_;
};

} // namespace proxygen::coro
