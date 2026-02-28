/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <utility>

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPHybridSource.h"

namespace proxygen::coro {

class ExpectContinueWrapperResponse : public HTTPSource {
 public:
  explicit ExpectContinueWrapperResponse(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      std::unique_ptr<HTTPMessage> consumedRequestHeader,
      HTTPSourceHolder bodyOnlySource,
      std::shared_ptr<HTTPHandler> nextHandler)
      : evb_(evb),
        ctx_(std::move(ctx)),
        requestForNextHandler_(),
        nextHandler_(std::move(nextHandler)) {
    auto hybridSource = new HTTPHybridSource(std::move(consumedRequestHeader),
                                             std::move(bodyOnlySource));
    hybridSource->setHeapAllocated();
    requestForNextHandler_.setSource(hybridSource);
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override;

  void stopReading(folly::Optional<const HTTPErrorCode> = folly::none) override;

 private:
  folly::coro::Task<void> ensureNextHandlerInvoked();
  folly::EventBase* evb_;
  HTTPSessionContextPtr ctx_;
  bool response100Written_{false};
  HTTPSourceHolder requestForNextHandler_;
  std::shared_ptr<HTTPHandler> nextHandler_;
  HTTPSourceHolder wrappedResponseHolder_;
};

class ExpectContinueWrapperHandler : public HTTPHandler {
 public:
  explicit ExpectContinueWrapperHandler(
      std::shared_ptr<HTTPHandler> nextHandler)
      : nextHandler_(std::move(nextHandler)) {
  }

  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) override;

 private:
  std::shared_ptr<HTTPHandler> nextHandler_;
};

} // namespace proxygen::coro
