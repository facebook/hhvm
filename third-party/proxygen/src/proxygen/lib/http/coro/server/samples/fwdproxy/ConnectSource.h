/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include <proxygen/lib/utils/ConditionalGate.h>

namespace proxygen::coro {

class ConnectSource : public HTTPSource {
 public:
  ConnectSource(std::unique_ptr<folly::coro::Transport> transport,
                HTTPSourceHolder requestSource)
      : transport_(std::move(transport)), reqSource_(std::move(requestSource)) {
    setHeapAllocated();
    gate_.then([this] {
      transport_->close();
      delete this;
    });
  }

  // Read from the HTTP request and write on the upstream transport
  folly::coro::Task<void> readRequestSendUpstream() {
    auto guard =
        folly::makeGuard([this] { gate_.set(Event::ReadRequestComplete); });
    bool eom = false;
    do {
      auto bodyEvent = co_await co_awaitTry(reqSource_.readBodyEvent(65535));
      if (bodyEvent.hasException()) {
        XLOG(ERR) << "read exception:" << bodyEvent.exception().what();
        // This will break the server read loop
        transport_->closeWithReset();
        co_return;
      }
      if (bodyEvent->eventType == HTTPBodyEvent::EventType::BODY &&
          !bodyEvent->event.body.empty()) {
        folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
        writeBuf.append(bodyEvent->event.body.move());
        XLOG(DBG2) << "Got " << writeBuf.chainLength() << " client bytes";
        try {
          co_await transport_->write(writeBuf,
                                     std::chrono::milliseconds(writeTimeout_));
        } catch (const std::exception& ex) {
          XLOG(ERR) << "Upstream write error: " << ex.what();
          transport_->closeWithReset();
          co_return;
        }
      } // else ignore any other events
      eom = bodyEvent->eom;
    } while (!eom);
    XLOG(DBG2) << "Client read finished";
    transport_->shutdownWrite();
  }

  // HTTPSource overrides

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    // Send 200 Connected
    auto resp = std::make_unique<HTTPMessage>();
    resp->setHTTPVersion(1, 1);
    resp->setStatusCode(200);
    resp->setStatusMessage("Connected");
    co_return HTTPHeaderEvent(std::move(resp), false);
  }

  // Read from the upstream socket and return as HTTPBodyEvents
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
    folly::IOBufQueue readBuf(folly::IOBufQueue::cacheChainLength());
    inRead_ = true;
    auto nread = co_await co_awaitTry(folly::coro::co_withCancellation(
        cancellationSource_.getToken(),
        transport_->read(readBuf, 1500, 4096, readTimeout_)));
    inRead_ = false;
    if (nread.hasException()) {
      XLOG(ERR) << "read error: " << nread.exception().what();
      gate_.set(Event::WriteResponseComplete);
      co_yield folly::coro::co_error(std::move(nread.exception()));
    }
    XLOG(DBG2) << "Got " << *nread << " server bytes";
    if (*nread == 0) {
      gate_.set(Event::WriteResponseComplete);
    }
    co_return HTTPBodyEvent(readBuf.move(), *nread == 0);
  }

  void stopReading(folly::Optional<const HTTPErrorCode>) override {
    if (inRead_) {
      cancellationSource_.requestCancellation();
    } else {
      gate_.set(Event::WriteResponseComplete);
    }
  }

  void setReadTimeout(std::chrono::milliseconds timeout) override {
    readTimeout_ = timeout;
  }
  void setWriteTimeout(std::chrono::milliseconds timeout) {
    writeTimeout_ = timeout;
  }

 private:
  std::unique_ptr<folly::coro::Transport> transport_;
  HTTPSourceHolder reqSource_;
  enum class Event { ReadRequestComplete, WriteResponseComplete };
  ConditionalGate<Event, 2> gate_;
  static constexpr auto kDefaultTimeout = std::chrono::milliseconds{5'000};
  std::chrono::milliseconds readTimeout_{kDefaultTimeout};
  std::chrono::milliseconds writeTimeout_{kDefaultTimeout};
  folly::CancellationSource cancellationSource_;
  bool inRead_{false};
};

} // namespace proxygen::coro
