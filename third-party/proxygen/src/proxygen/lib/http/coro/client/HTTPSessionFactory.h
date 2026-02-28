/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <utility>

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/client/HTTPClient.h"

namespace proxygen::coro {

/* Abstract class for making a session */
class HTTPSessionFactory {
 public:
  virtual ~HTTPSessionFactory() = default;

  struct GetSessionResult {
    HTTPCoroSession::RequestReservation reservation;
    HTTPCoroSession* session;
    HTTPSessionContextPtr ctx;
    GetSessionResult(HTTPCoroSession::RequestReservation res,
                     HTTPCoroSession* sess)
        : reservation(std::move(res)),
          session(sess),
          ctx(sess->acquireKeepAlive()) {
    }
    GetSessionResult(GetSessionResult&&) = default;
    GetSessionResult& operator=(GetSessionResult&&) = default;
  };

  static constexpr std::chrono::seconds kDefaultConnectTimeout{2};

  virtual folly::coro::Task<GetSessionResult> getSessionWithReservation(
      std::string host,
      uint16_t port,
      bool isSecure,
      std::chrono::milliseconds connectTimeout = kDefaultConnectTimeout,
      folly::Optional<std::string> serverAddress = folly::none) = 0;

  // Return true if the sessions returned by this factory require absolute URLs
  // eg: they go via a forward proxy
  [[nodiscard]] virtual bool requiresAbsoluteURLs() const {
    return false;
  }
};

class SimpleHTTPSessionFactory : public HTTPSessionFactory {
 public:
  SimpleHTTPSessionFactory() = delete;
  explicit SimpleHTTPSessionFactory(folly::EventBase* evb) : evb_(evb) {
  }
  ~SimpleHTTPSessionFactory() override = default;

  folly::coro::Task<GetSessionResult> getSessionWithReservation(
      std::string host,
      uint16_t port,
      bool isSecure,
      std::chrono::milliseconds connectTimeout,
      folly::Optional<std::string> serverAddress = folly::none) override {
    static constexpr std::chrono::seconds kDefaultStreamReadTimeout{5};
    auto session =
        co_await HTTPClient::getHTTPSession(evb_,
                                            host,
                                            port,
                                            isSecure,
                                            false,
                                            connectTimeout,
                                            kDefaultStreamReadTimeout,
                                            "" /* clientCertPath */,
                                            "" /* clientKeyPath */,
                                            serverAddress);
    auto reservation = session->reserveRequest();
    if (reservation.hasException()) {
      co_yield folly::coro::co_error(std::move(reservation.exception()));
    }
    co_return GetSessionResult(std::move(*reservation), session);
  }

 private:
  folly::EventBase* evb_;
};

} // namespace proxygen::coro
