/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/HTTPClientConnectionCache.h"
#include "proxygen/lib/http/coro/client/CoroDNSResolver.h"
#include "proxygen/lib/http/coro/client/HTTPClient.h"
#include <folly/logging/xlog.h>

using ConnectionParams = proxygen::coro::HTTPCoroConnector::ConnectionParams;
using SessionParams = proxygen::coro::HTTPCoroConnector::SessionParams;

namespace {

constexpr uint32_t kMaxConnections = 100;
constexpr std::chrono::seconds kConnectTimeout(3);
constexpr uint32_t kConnectAttempts = 1;
constexpr std::chrono::seconds kMaxAge(0);
constexpr size_t kMaxPoolsToTraverseDuringReap = 5;
constexpr bool kEnableSslSessionCaching = true;

std::string makePoolKey(folly::StringPiece address,
                        uint16_t port,
                        bool isSecure,
                        const ConnectionParams *connParams) {
  auto id =
      folly::to<std::string>("http", (isSecure ? "s" : ""), address, ":", port);
  if (!connParams) {
    return id;
  }

  // connParams != nullptr indicates we should hash combine member fields to
  // create a unique identified for this session pool
  size_t hash = folly::hash::hash_combine(
      connParams->sslContext.get(),
      connParams->fizzContextAndVerifier.fizzContext.get(),
      connParams->fizzContextAndVerifier.fizzCertVerifier.get());
  folly::ByteRange br(reinterpret_cast<const uint8_t *>(&hash), sizeof(size_t));
  return folly::to<std::string>(folly::hexlify(br), "/", id);
}

} // namespace

namespace proxygen::coro {

HTTPClientConnectionCache::~HTTPClientConnectionCache() {
  eventBase_.runImmediatelyOrRunInEventBaseThreadAndWait([this]() { drain(); });
}

void HTTPClientConnectionCache::drain() {
  eventBase_.checkIsInEventBaseThread();
  cancellationSource_.requestCancellation();
  proxyPool_.reset();
  pools_.clear();
}

folly::coro::Task<HTTPSessionFactory::GetSessionResult>
HTTPClientConnectionCache::getSessionWithReservation(
    std::string urlStr,
    std::chrono::milliseconds connectTimeout,
    const HTTPCoroConnector::ConnectionParams *connParams) {
  URL url(urlStr);
  if (!url.isValid() || !url.hasHost()) {
    return folly::coro::makeErrorTask<HTTPCoroSessionPool::GetSessionResult>(
        std::runtime_error("invalid url"));
  }

  return getSessionWithReservation(url.getHost(),
                                   url.getPort(),
                                   url.getScheme() == "https",
                                   connectTimeout,
                                   connParams,
                                   folly::none);
}

folly::coro::Task<HTTPSessionFactory::GetSessionResult>
HTTPClientConnectionCache::getSessionWithReservation(
    std::string host,
    uint16_t port,
    bool isSecure,
    std::chrono::milliseconds connectTimeout,
    folly::Optional<std::string> serverAddress) {
  return getSessionWithReservation(std::move(host),
                                   port,
                                   isSecure,
                                   connectTimeout,
                                   nullptr,
                                   std::move(serverAddress));
}

folly::coro::Task<HTTPSessionFactory::GetSessionResult>
HTTPClientConnectionCache::getSessionWithReservation(
    std::string host,
    uint16_t port,
    bool isSecure,
    std::chrono::milliseconds connectTimeout,
    const HTTPCoroConnector::ConnectionParams *connParams,
    folly::Optional<std::string> serverAddress) {
  const auto &reqCancelToken =
      co_await folly::coro::co_current_cancellation_token;
  auto drainCancelToken = cancellationSource_.getToken();

  std::string addressStr;
  if (proxyPool_) {
    addressStr = host;
  } else if (serverAddress.has_value()) {
    addressStr = serverAddress.value();
  } else {
    auto serverAddresses = co_await CoroDNSResolver::resolveHost(
        &eventBase_, host, connectTimeout);
    // TODO: support happy eyeballs
    addressStr = serverAddresses.primary.getAddressStr();
  }

  if (reqCancelToken.isCancellationRequested()) {
    co_yield folly::coro::co_error(
        HTTPError(HTTPErrorCode::CORO_CANCELLED, "Cancelled"));
  }
  if (drainCancelToken.isCancellationRequested()) {
    co_yield folly::coro::co_error(HTTPCoroSessionPool::Exception(
        HTTPCoroSessionPool::Exception::Type::Draining, "Pool is draining"));
  }

  // Reclaim some memory by reaping empty pools
  reapEmptyPools(kMaxPoolsToTraverseDuringReap);

  // get or create a session pool to this destination address
  auto &pool = getPool(host, addressStr, port, isSecure, connParams);

  /**
   * Obtain a session from the pool, propagate errors
   *
   * HTTPCoroSessionPool::getSessionWithReservation() coroutine does not need to
   * directly depend on drainCancelToken (cancelled via ::drain), since all
   * pool are destroyed anyways in ::drain and the HTTPCoroSessionPool
   * destructor cancels the pool's cancellation source. This allows
   * HTTPCoroSessionPool to differentiate between drain and cancel exceptions.
   */
  co_return co_await pool.getSessionWithReservation();
}

void HTTPClientConnectionCache::reapEmptyPools(size_t maxPoolsToTraverse) {
  for (auto it = pools_.cend(); maxPoolsToTraverse && it != pools_.cbegin();
       --maxPoolsToTraverse) {
    --it;
    if (it->second->empty()) {
      it = pools_.erase(it);
    }
  }
}

proxygen::coro::HTTPCoroSessionPool &HTTPClientConnectionCache::getPool(
    folly::StringPiece host,
    folly::StringPiece address,
    uint16_t port,
    bool isSecure,
    const HTTPCoroConnector::ConnectionParams *maybeConnParams) {
  if (usingProxy() && !useConnectForProxy_) {
    XLOG(DBG4) << "Not using CONNECT";
    // The caller will make a GET request via the proxy.
    return *proxyPool_;
  }
  auto key = makePoolKey(address, port, isSecure, maybeConnParams);
  auto it = pools_.find(key);
  auto sni = folly::IPAddress::validate(host) ? "" : host;
  if (it == pools_.end()) {
    XLOG(DBG4) << "Making a new pool for key: " << key;
    HTTPCoroConnector::ConnectionParams connParams;
    if (maybeConnParams || connParams_) {
      connParams = maybeConnParams ? *maybeConnParams : *connParams_;
      if (isSecure) {
        connParams.serverName = std::string(sni);
      } else {
        connParams.sslContext = nullptr;
        connParams.fizzContextAndVerifier.fizzContext = nullptr;
      }
    } else {
      // Use OpenSSL when connecting via proxy, target may not support TLS 1.3
      connParams = HTTPClient::getConnParams(
          isSecure ? (usingProxy() ? HTTPClient::SecureTransportImpl::TLS
                                   : HTTPClient::SecureTransportImpl::FIZZ)
                   : HTTPClient::SecureTransportImpl::NONE,
          sni);
      if (isSecure && connParams.fizzContextAndVerifier.fizzContext &&
          !connParams.fizzContextAndVerifier.fizzContext->getPskCache()) {
        auto newFizzCtx = std::make_shared<fizz::client::FizzClientContext>(
            *connParams.fizzContextAndVerifier.fizzContext);
        newFizzCtx->setPskCache(pskCache_);
        connParams.fizzContextAndVerifier.fizzContext = std::move(newFizzCtx);
      }
    }
    auto poolParams =
        poolParams_.value_or(proxygen::coro::HTTPCoroSessionPool::PoolParams{
            kMaxConnections,
            kConnectAttempts,
            kEnableSslSessionCaching,
            kConnectTimeout,
            kMaxAge});
    auto sessParams = sessionParams_.value_or(HTTPClient::getSessionParams());
    std::unique_ptr<HTTPCoroSessionPool> pool;
    if (proxyPool_) {
      pool = std::make_unique<HTTPCoroSessionPool>(&eventBase_,
                                                   address.str(),
                                                   port,
                                                   proxyPool_,
                                                   poolParams,
                                                   std::move(connParams),
                                                   std::move(sessParams));
    } else {
      pool = std::make_unique<HTTPCoroSessionPool>(&eventBase_,
                                                   address.str(),
                                                   port,
                                                   poolParams,
                                                   std::move(connParams),
                                                   std::move(sessParams));
    }
    auto res = pools_.insert(key, std::move(pool));
    it = res.first;
  }
  return *it->second;
}

bool HTTPClientConnectionCache::poolExists(folly::StringPiece address,
                                           uint16_t port,
                                           bool isSecure,
                                           const ConnectionParams *connParams) {
  auto key = makePoolKey(address, port, isSecure, connParams);
  return pools_.exists(key);
}

} // namespace proxygen::coro
