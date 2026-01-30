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
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/client/HTTPSessionFactory.h"

using RedirectCallback = std::function<void(const std::string&)>;

namespace proxygen::coro {

class HTTPRedirectHandler {
 public:
  static constexpr std::chrono::seconds kDefaultConnectTimeout{2};

  explicit HTTPRedirectHandler(
      std::shared_ptr<HTTPSessionFactory> sessionFactory,
      uint16_t maxRedirects = 1,
      std::chrono::milliseconds connectTimeout = kDefaultConnectTimeout,
      folly::Optional<std::chrono::milliseconds> readTimeout = folly::none)
      : reqFilter_(*this),
        respFilter_(*this),
        sessionFactory_(std::move(sessionFactory)),
        connectTimeout_(connectTimeout),
        readTimeout_(std::move(readTimeout)),
        maxRedirects_(maxRedirects) {
  }

  class Exception : public std::runtime_error {
   public:
    enum class Type { InvalidRedirect, MaxRedirects, UnsupportedScheme };
    explicit Exception(Type t, const std::string& msg)
        : std::runtime_error(msg), type(t) {
    }

    Type type;
  };

  HTTPSourceFilter* getRequestFilter(HTTPSourceHolder source) {
    reqFilter_.setSource(source.release());
    return &reqFilter_;
  }

  HTTPSourceFilter* getResponseFilter(HTTPSourceHolder source) {
    respFilter_.setSource(source.release());
    return &respFilter_;
  }

  // Set a callback to run each time we have identified a new URL to redirect
  // to. URL is passed as const std::string&
  void setRedirectCallback(RedirectCallback&& callback) {
    redirectCallback_ = std::move(callback);
  }

 private:
  class RequestFilter : public HTTPSourceFilter {
   public:
    explicit RequestFilter(HTTPRedirectHandler& handler) : handler_(handler) {
    }

    folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
      auto headerEvent = co_await readHeaderEventImpl();
      // Save a copy in case we need to redirecit
      handler_.request_ = std::make_unique<HTTPMessage>(*headerEvent.headers);
      co_return headerEvent;
    }
    folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
      auto bodyEvent = co_await readBodyEventImpl(max);
      if (bodyEvent.eventType == HTTPBodyEvent::BODY) {
        handler_.requestBody_.append(bodyEvent.event.body.clone());
      }
      co_return bodyEvent;
    }

   private:
    HTTPRedirectHandler& handler_;
  };

  class ResponseFilter : public HTTPSourceFilter {
   public:
    explicit ResponseFilter(HTTPRedirectHandler& handler) : handler_(handler) {
    }

    folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
    // default readBodyEvent
    void stopReading(folly::Optional<const HTTPErrorCode> error) override;

   private:
    HTTPRedirectHandler& handler_;
    folly::CancellationSource cancellationSource_;
  };

  [[nodiscard]] folly::Optional<URL> getRedirectDestination(
      const std::string& location) const;
  void prepareRequest(const std::string& newUrlStr,
                      const URL& newUrl,
                      uint16_t statusCode);
  folly::coro::Task<void> redirect(URL url);

  RequestFilter reqFilter_;
  ResponseFilter respFilter_;
  std::unique_ptr<HTTPMessage> request_;
  BufQueue requestBody_;
  std::shared_ptr<HTTPSessionFactory> sessionFactory_;
  std::chrono::milliseconds connectTimeout_;
  folly::Optional<std::chrono::milliseconds> readTimeout_;
  uint16_t maxRedirects_{1};

  // Funciton pointer to run each time we find a new URL to redirect to.
  RedirectCallback redirectCallback_ = nullptr;
};

} // namespace proxygen::coro
