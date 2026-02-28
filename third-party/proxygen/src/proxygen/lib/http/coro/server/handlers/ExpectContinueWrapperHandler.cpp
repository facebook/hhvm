/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/server/handlers/ExpectContinueWrapperHandler.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include <proxygen/lib/http/HTTPCommonHeaders.h>

#include <folly/logging/xlog.h>
#include <string_view>

namespace proxygen::coro {

constexpr folly::StringPiece k100ContinueExpectation = "100-continue";
constexpr std::string_view kContinueMessage = "Continue";
constexpr std::string_view kExpectationFailedMessage = "Expectation Failed";
constexpr uint16_t kContinueStatusCode = 100;
constexpr uint16_t kExpectationFailedStatusCode = 417;

folly::coro::Task<HTTPHeaderEvent>
ExpectContinueWrapperResponse::readHeaderEvent() {
  if (!response100Written_) {
    XLOG(DBG8)
        << "ExpectContinueWrapper sending 100-continue to fulfill expectation";
    response100Written_ = true;
    auto msg = std::make_unique<HTTPMessage>();
    msg->setStatusCode(kContinueStatusCode);
    msg->setStatusMessage(kContinueMessage);
    msg->setHTTPVersion(1, 1);
    HTTPHeaderEvent event(std::move(msg), false);
    co_return event;
  } else {
    co_await ensureNextHandlerInvoked();
    auto event = co_await wrappedResponseHolder_.readHeaderEvent();
    auto guard = folly::makeGuard(lifetime(event));
    co_return event;
  }
}

folly::coro::Task<HTTPBodyEvent> ExpectContinueWrapperResponse::readBodyEvent(
    uint32_t max) {
  co_await ensureNextHandlerInvoked();
  auto event = co_await wrappedResponseHolder_.readBodyEvent(max);
  auto guard = folly::makeGuard(lifetime(event));
  co_return event;
}

void ExpectContinueWrapperResponse::stopReading(
    folly::Optional<const HTTPErrorCode>) {
  if (heapAllocated_) {
    delete this;
  }
}

folly::coro::Task<void>
ExpectContinueWrapperResponse::ensureNextHandlerInvoked() {
  if (!bool(wrappedResponseHolder_)) {
    XLOG(DBG8) << "ExpectContinueWrapper invoking wrapped handler";
    wrappedResponseHolder_ = co_await nextHandler_->handleRequest(
        evb_, ctx_, std::move(requestForNextHandler_));
  }
}

folly::coro::Task<HTTPSourceHolder> ExpectContinueWrapperHandler::handleRequest(
    folly::EventBase* evb,
    HTTPSessionContextPtr ctx,
    HTTPSourceHolder requestSource) {
  auto headerEvent = co_await requestSource.readHeaderEvent();
  XCHECK(headerEvent.headers);
  auto headers = headerEvent.headers->getHeaders();
  if (headers.exists(HTTP_HEADER_EXPECT)) {
    auto& expectVal = headers.getSingleOrEmpty(HTTP_HEADER_EXPECT);
    if (k100ContinueExpectation.equals(expectVal,
                                       folly::AsciiCaseInsensitive{})) {
      // Request has an expectation with the value 100-continue
      // We need to wrap the next handler in an ExpectContinueWrapperResponse
      // to inject the 100 Continue response
      XLOG(DBG8) << "ExpectContinueWrapper found 100-continue expectation";
      headerEvent.headers->getHeaders().remove(HTTP_HEADER_EXPECT);
      auto expectWrapperResponse =
          new ExpectContinueWrapperResponse(evb,
                                            ctx,
                                            std::move(headerEvent.headers),
                                            std::move(requestSource),
                                            nextHandler_);
      expectWrapperResponse->setHeapAllocated();
      co_return expectWrapperResponse;
    } else {
      // Invalid expectation. Send a response failing this request.
      XLOG(DBG8)
          << "ExpectContinueWrapper returning 417 for invalid expectation: "
          << expectVal;
      co_return HTTPFixedSource::makeFixedResponse(
          kExpectationFailedStatusCode, std::string(kExpectationFailedMessage));
    }
  } else {
    // This request does not have an expectation. Bypass the wrapper
    // and handle this request directly using the next handler.

    // Combine already consumed headers with the requestSource in a new
    // HybridSource
    auto hybridSource = new HTTPHybridSource(std::move(headerEvent.headers),
                                             std::move(requestSource));
    hybridSource->setHeapAllocated();
    co_return co_await nextHandler_->handleRequest(evb, ctx, hybridSource);
  }
}

} // namespace proxygen::coro
