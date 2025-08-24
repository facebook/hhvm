/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/HTTPMessage.h"
#include "proxygen/lib/http/codec/test/TestUtils.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/HTTPSourceHolder.h"
#include <folly/coro/Sleep.h>
#include <folly/io/IOBufQueue.h>

namespace proxygen::coro::test {

// ErrorSource is a source that throws an exception in the middle of the
// 'readBodyEvent' routine
struct ErrorSource : public HTTPSourceFilter {
  explicit ErrorSource(std::string body,
                       bool client,
                       uint32_t bytesTillTheError,
                       uint32_t limit = 10)
      : HTTPSourceFilter(
            HTTPFixedSource::makeFixedResponse(200, std::move(body))),
        client_(client),
        bytesTillTheError_(bytesTillTheError),
        limit_(limit) {
    setHeapAllocated();
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    std::unique_ptr<HTTPHeaderEvent> eventPtr;
    if (client_) {
      auto req = getPostRequest(bytesTillTheError_ + 1);
      req.setURL<std::string>(folly::to<std::string>("/incompleteBody"));
      eventPtr = std::make_unique<HTTPHeaderEvent>(
          std::make_unique<HTTPMessage>(std::move(req)), false);
    } else {
      eventPtr =
          std::make_unique<HTTPHeaderEvent>(co_await readHeaderEventImpl(true));
    }

    auto event = std::move(*eventPtr);
    auto guard = folly::makeGuard(lifetime(event));
    co_return event;
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
    auto bodyEvent = co_await readBodyEventImpl(
        std::min(static_cast<uint32_t>(limit_), max));
    if (bodyEvent.eventType == HTTPBodyEvent::EventType::BODY) {
      bytesTillTheError_ -= bodyEvent.event.body.chainLength();
    }

    if (bodyEvent.eom || bytesTillTheError_ <= 0) {
      stopReading();
      co_yield folly::coro::co_error(proxygen::coro::HTTPError(
          HTTPErrorCode::INTERNAL_ERROR, "Failed to read body"));
    }

    auto guard = folly::makeGuard(lifetime(bodyEvent));
    co_return bodyEvent;
  }

  bool client_;
  int64_t bytesTillTheError_;
  uint32_t limit_;
};

/* OnEOMSource is a source filter that runs a user callback before EOM */
class OnEOMSource : public HTTPSourceFilter {
 public:
  using CallbackReturn = folly::coro::Task<folly::Optional<HTTPError>>;

  OnEOMSource(HTTPSource *source, std::function<CallbackReturn()> eomCallback)
      : HTTPSourceFilter(source), eomCallback_(eomCallback) {
    setHeapAllocated();
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override {
    auto bodyEvent = co_await co_awaitTry(readBodyEventImpl(max));
    auto guard = folly::makeGuard(lifetime(bodyEvent));
    if (bodyEvent.hasException()) {
      co_yield folly::coro::co_error(getHTTPError(bodyEvent));
    }
    if (bodyEvent->eom) {
      error_ = co_await eomCallback_();
      if (error_) {
        // doesn't affect lifetime
        co_yield folly::coro::co_error(*error_);
      }
    }

    co_return bodyEvent;
  }

  folly::Optional<HTTPError> error_;
  std::function<CallbackReturn()> eomCallback_;
};

// An HTTP source that will hang in readBodyEvent
class TimeoutSource : public HTTPSource {
 public:
  explicit TimeoutSource(std::unique_ptr<HTTPMessage> msg,
                         bool timeoutHeaders = false,
                         bool errorOnCancel = true,
                         bool heapAllocated = true)
      : msg_(std::move(msg)),
        timeoutHeaders_(timeoutHeaders),
        errorOnCancel_(errorOnCancel) {
    if (heapAllocated) {
      setHeapAllocated();
    }
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    if (timeoutHeaders_) {
      co_await folly::coro::sleepReturnEarlyOnCancel(std::chrono::seconds(60));
      if (errorOnCancel_) {
        if (heapAllocated_) {
          delete this;
        }
        co_yield folly::coro::co_error(
            HTTPError(HTTPErrorCode::READ_TIMEOUT, ""));
      }
    }
    co_return HTTPHeaderEvent(std::move(msg_), false);
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
    SCOPE_EXIT {
      if (heapAllocated_) {
        delete this;
      }
    };
    if (!timeoutHeaders_) {
      co_await folly::coro::sleepReturnEarlyOnCancel(std::chrono::seconds(60));
      if (errorOnCancel_) {
        co_yield folly::coro::co_error(
            HTTPError(HTTPErrorCode::READ_TIMEOUT, ""));
      }
    }

    co_return HTTPBodyEvent(std::unique_ptr<folly::IOBuf>(nullptr), true);
  }

  void stopReading(
      folly::Optional<const HTTPErrorCode> = folly::none) override {
    if (heapAllocated_) {
      delete this;
    }
  }

  std::unique_ptr<HTTPMessage> msg_;
  bool timeoutHeaders_{false};
  bool errorOnCancel_{true};
};

class EchoBodySource : public HTTPSourceFilter {
 public:
  explicit EchoBodySource(HTTPSourceHolder requestSource,
                          uint16_t statusCode,
                          bool eom,
                          std::map<std::string, std::string> headers,
                          bool abortBody)
      : HTTPSourceFilter(requestSource.release()),
        eom_(eom),
        abortBody_(abortBody) {
    setHeapAllocated();
    response_ = std::make_unique<HTTPMessage>();
    response_->setHTTPVersion(1, 1);
    response_->setStatusCode(statusCode);
    for (auto nv : headers) {
      response_->getHeaders().add(nv.first, nv.second);
    }
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    HTTPHeaderEvent event(std::move(response_), eom_);
    if (eom_) {
      delete this;
    }
    co_return event;
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
    if (abortBody_) {
      delete this;
      co_yield folly::coro::co_error(
          HTTPError(HTTPErrorCode::CANCEL, "cancel"));
    }
    auto bodyEvent = co_await co_awaitTry(readBodyEventImpl(max));
    if (!*this) {
      delete this;
    }
    co_return bodyEvent;
  }

  void setReadTimeout(std::chrono::milliseconds) override {
  }

  std::unique_ptr<HTTPMessage> response_;
  bool eom_{false};
  bool abortBody_{false};
};

class ByteEventFilter : public HTTPSourceFilter {
 public:
  ByteEventFilter(uint8_t headerEvents,
                  uint8_t bodyEvents,
                  HTTPByteEventCallbackPtr callback)
      : headerEvents_(headerEvents),
        bodyEvents_(bodyEvents),
        callback_(std::move(callback)) {
    setHeapAllocated();
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    auto ev = co_await HTTPSourceFilter::readHeaderEventImpl();
    HTTPByteEventRegistration reg;
    reg.events = headerEvents_;
    reg.callback = callback_;
    ev.byteEventRegistrations.emplace_back(std::move(reg));
    lifetime(ev)();
    co_return ev;
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
    auto ev = co_await HTTPSourceFilter::readBodyEventImpl(max);
    HTTPByteEventRegistration reg;
    reg.events = bodyEvents_;
    reg.callback = callback_;
    ev.byteEventRegistrations.emplace_back(std::move(reg));
    lifetime(ev)();
    co_return ev;
  }

  void stopReading(
      folly::Optional<const HTTPErrorCode> = folly::none) override {
    if (heapAllocated_) {
      delete this;
    }
  }

 private:
  uint8_t headerEvents_;
  uint8_t bodyEvents_;
  HTTPByteEventCallbackPtr callback_;
};

class YieldExceptionSource : public HTTPSource {
 public:
  enum Stage : uint8_t { HeaderEvent, BodyEvent };
  enum MessageType : uint8_t { Request, Response };
  using co_error = folly::coro::co_error;

  YieldExceptionSource(Stage stage, MessageType type)
      : stage_(stage), type_(type) {
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    if (stage_ == HeaderEvent) {
      co_yield co_error(HTTPError(HTTPErrorCode::INTERNAL_ERROR, "exception"));
    }
    co_return HTTPHeaderEvent(type_ == Request ? proxygen::makeGetRequest()
                                               : proxygen::makeResponse(200),
                              /*inEOM=*/false);
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override {
    XCHECK(stage_ == BodyEvent);
    [[maybe_unused]] auto guard = folly::makeGuard([this] {
      if (heapAllocated_) {
        delete this;
      }
    });
    co_yield co_error(HTTPError(HTTPErrorCode::INTERNAL_ERROR, "exception"));
  }

  void stopReading(
      folly::Optional<const HTTPErrorCode> = folly::none) override {
  }

 private:
  const Stage stage_;
  const MessageType type_;
};

/**
 * Propagates a body in a fixed size chunks. Useful if you want to test
 * aborting reading the body or any behavior requiring multiple readBodyEvent.
 */
class ChunkedBodySource : public HTTPSourceFilter {
 public:
  ChunkedBodySource(std::string body, uint32_t chunkSize)
      : HTTPSourceFilter(
            HTTPFixedSource::makeFixedResponse(200, std::move(body))),
        chunkSize_(chunkSize) {
    setHeapAllocated();
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override {
    auto bodyEvent = co_await readBodyEventImpl(chunkSize_);
    auto guard = folly::makeGuard(lifetime(bodyEvent));
    co_return bodyEvent;
  }

 private:
  uint32_t chunkSize_;
};

} // namespace proxygen::coro::test
