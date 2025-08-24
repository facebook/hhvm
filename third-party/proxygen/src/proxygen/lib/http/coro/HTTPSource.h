/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Function.h>
#include <folly/ScopeGuard.h>
#include <folly/coro/Task.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/coro/HTTPError.h>
#include <proxygen/lib/http/coro/HTTPEvents.h>

namespace proxygen::coro {

/*
 * Abstract class for producing an HTTP message.
 *
 * The normal flow is as follows:
 *
 * do {
 *   headerEvent = co_await co_awaitTry(source.readHeaderEvent());
 *   if (headerEvent.hasException()) {
 *     // handle error
 *     return;
 *   }
 *   // use headers
 * } while (!headerEvent->final);
 *
 * // Note: while loop is unnecessary for requests, the first header event
 * // will be final
 *
 * bool eom = headerEvent->eom;
 * while (!eom) {
 *   bodyEvent = co_await co_awaitTry(source.readBodyEvent(max));
 *   if (bodyEvent.hasException()) {
 *     // handle error
 *     return;
 *   }
 *   eom = bodyEvent->eom;
 *   switch (bodyEvent.eventType) {
 *     case BODY:
 *       // use body
 *     // other cases, PUSH_PROMISE, TRAILERS
 *   }
 * }
 *
 * Read events until you read an error, or an EOM.  Once you read an error or
 * EOM, assume your source is gone.  If you are implenting a source, you should
 * free all resources once you have returned error or EOM.
 *
 * If you wish to stop reading from a source before reading error or EOM,
 * you must call stopReading().
 *
 * To return an error from a source, use HTTPError - see HTTPError.h for details
 *   co_yield folly:coro::co_error(HTTPError(HTTPErrorCode, msg))
 *
 * Some sources could be allocated on coroutine stacks, or belong to bigger
 * objects, so do not need deallocation.  If you heap allocate a source, you
 * can use the lifetime function with folly::makeGuard to help ensure timely
 * destruction of your source.
 */
class HTTPSource {
 public:
  virtual ~HTTPSource() {
  }

  virtual folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() = 0;

  virtual folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) = 0;

  // Call stopReading if you do not intend to read the object to completion
  virtual void stopReading(
      folly::Optional<const HTTPErrorCode> = folly::none) = 0;

  virtual folly::Optional<uint64_t> getStreamID() const {
    return folly::none;
  }

  virtual void setReadTimeout(std::chrono::milliseconds /*timeout*/) {
  }

  void setHeapAllocated() {
    heapAllocated_ = true;
  }

 protected:
  // Lambda helpers to pass to folly::makeGuard in derived implementations
  // of readHeaderEvent and readBodyEvent
  folly::Function<void()> lifetime(folly::Try<HTTPHeaderEvent>& event) {
    return [this, &event] {
      if (heapAllocated_ && (event.hasException() || event->eom)) {
        delete this;
      }
    };
  }
  folly::Function<void()> lifetime(folly::Try<HTTPBodyEvent>& event) {
    return [this, &event] {
      if (heapAllocated_ && (event.hasException() || event->eom)) {
        delete this;
      }
    };
  }
  folly::Function<void()> lifetime(HTTPHeaderEvent& event) {
    return [this, &event] {
      if (heapAllocated_ && event.eom) {
        delete this;
      }
    };
  }
  folly::Function<void()> lifetime(HTTPBodyEvent& event) {
    return [this, &event] {
      if (heapAllocated_ && event.eom) {
        delete this;
      }
    };
  }

  bool heapAllocated_{false};
};

template <typename T>
HTTPError getHTTPError(const folly::Try<T>& tryEvent) {
  XCHECK(tryEvent.hasException());
  auto httpErr = tryEvent.template tryGetExceptionObject<HTTPError>();
  if (httpErr) {
    return std::move(*httpErr);
  }
  auto ex = tryEvent.tryGetExceptionObject();
  XCHECK(ex);
  return HTTPError(HTTPErrorCode::INTERNAL_ERROR, ex->what());
}

} // namespace proxygen::coro
