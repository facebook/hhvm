/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPStreamSourceHolder.h"
#include "proxygen/lib/http/coro/client/HTTPCoroConnector.h"
#include "proxygen/lib/http/coro/server/ScopedHTTPServer.h"
#include "proxygen/lib/http/coro/transport/HTTPConnectAsyncTransport.h"
#include "proxygen/lib/http/coro/transport/HTTPConnectTransport.h"
#include <proxygen/lib/http/codec/test/TestUtils.h>

#include <folly/Synchronized.h>
#include <folly/coro/Sleep.h>
#include <folly/portability/GTest.h>

namespace {
constexpr char kAuthority[] = "example.com:443";
}

namespace proxygen::coro {

class ConnectHandler : public HTTPHandler {
 public:
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* eventBase,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) override {
    // numIOThreads = 1; only one evb ok to store
    this->evb = eventBase;
    peerAddr = ctx->getPeerAddress();
    auto headerEvent = co_await requestSource.readHeaderEvent();
    auto resp = std::make_unique<HTTPMessage>();
    EXPECT_EQ(
        headerEvent.headers->getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST),
        kAuthority);
    resp->getHeaders().set("X-Connected-To", "1.2.3.4");
    resp->setStatusCode(200);
    resp->setHTTPVersion(1, 1);
    auto respSourcePtr = HTTPStreamSourceHolder::make(eventBase);
    auto respSource = respSourcePtr->get();
    // 1xx response, because we can
    respSource->headers(makeResponse(107));
    if (headerEvent.headers->getHeaders().exists("Slow")) {
      co_await folly::coro::sleep(std::chrono::milliseconds(500));
    }
    if (headerEvent.headers->getHeaders().exists("Fail")) {
      respSource->headers(makeResponse(500), /*eom=*/true);
    } else {
      EXPECT_EQ(headerEvent.headers->getHeaders().getSingleOrEmpty("Foo"),
                "Bar");
      respSource->headers(std::move(resp), false);
    }
    curRespSource = respSource;
    co_withExecutor(eventBase,
                    handleConnect(std::move(requestSource), respSourcePtr))
        .start();
    respSourcePtr->start();
    co_return respSource;
  }

  folly::coro::Task<void> handleConnect(
      HTTPSourceHolder reqSource,
      std::shared_ptr<HTTPStreamSourceHolder> respSourcePtr) {
    SCOPE_EXIT {
      curRespSource = nullptr;
    };
    bool eom = false;
    if (timeout.count() > 0) {
      co_await folly::coro::sleep(timeout);
    }
    do {
      auto bodyEvent = co_await co_awaitTry(reqSource.readBodyEvent());
      if (bodyEvent.hasValue() &&
          bodyEvent->eventType == HTTPBodyEvent::EventType::SUSPEND) {
        co_await std::move(bodyEvent->event.resume);
        continue;
      }
      auto respSource = respSourcePtr->get();
      if (!respSource) {
        break;
      }
      exceptionExpected.withLock([&bodyEvent](auto& exceptionExpected) {
        if (exceptionExpected) {
          EXPECT_EQ(bodyEvent.hasException(), *exceptionExpected);
        }
      });
      if (bodyEvent.hasException()) {
        co_yield folly::coro::co_error(std::move(bodyEvent.exception()));
      }
      eom = bodyEvent->eom || (bodyEvent->event.body.front()->toString().rfind(
                                   "write_fin", 0) == 0);
      if (bodyEvent->eventType == HTTPBodyEvent::BODY) {
        respSource->body(bodyEvent->event.body.move(), 0, eom);
      }
    } while (!eom);
    curRespSource = nullptr;
  }

  void resetExceptionExpected() {
    exceptionExpected.withLock(
        [](auto& exceptionExpected) { exceptionExpected.reset(); });
  }

  void expectExceptions() {
    exceptionExpected.withLock(
        [](auto& exceptionExpected) { exceptionExpected = true; });
  }

  HTTPStreamSource* curRespSource{nullptr};
  folly::EventBase* evb{nullptr};
  folly::SocketAddress peerAddr;
  std::chrono::milliseconds timeout{0};

 private:
  folly::Synchronized<std::optional<bool>, std::mutex> exceptionExpected{false};
};

} // namespace proxygen::coro
