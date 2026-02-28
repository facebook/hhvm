/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transport/rocket/client/RequestContext.h>

#include <glog/logging.h>

#include <fmt/core.h>
#include <folly/Likely.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Assume.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/client/RequestContextQueue.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

using apache::thrift::transport::TTransportException;

namespace apache::thrift::rocket {

folly::Try<void> RequestContext::waitForWriteToComplete() {
  baton_.wait();
  return waitForWriteToCompleteResult();
}

void RequestContext::waitForWriteToCompleteSchedule(
    folly::fibers::Baton::Waiter* waiter) {
  baton_.setWaiter(*waiter);
}

folly::Try<void> RequestContext::waitForWriteToCompleteResult() {
  switch (state_) {
    case State::COMPLETE:
      if (responsePayload_.hasException()) {
        return folly::Try<void>(std::move(responsePayload_.exception()));
      }
      return {};

    case State::DEFERRED_INIT:
    case State::WRITE_NOT_SCHEDULED:
    case State::WRITE_SCHEDULED:
    case State::WRITE_SENDING:
    case State::WRITE_SENT:
      LOG(FATAL) << fmt::format(
          "Returned from Baton::wait() with unexpected state {} in {}",
          static_cast<int>(state_),
          __func__);
  }

  folly::assume_unreachable();
}

folly::Try<Payload> RequestContext::waitForResponse(
    std::chrono::milliseconds timeout) {
  CHECK(folly::fibers::onFiber());

  // The request timeout is scheduled only after the write to the socket
  // begins.
  folly::fibers::Baton::TimeoutHandler timeoutHandler;
  setTimeoutInfo(
      *folly::fibers::FiberManager::getFiberManagerUnsafe()
           ->loopController()
           .timer(),
      timeoutHandler,
      timeout);
  baton_.wait(timeoutHandler);
  return std::move(*this).getResponse();
}

folly::Try<Payload> RequestContext::getResponse() && {
  switch (state_) {
    case State::WRITE_SENDING:
      // Client timeout fired before writeSuccess()/writeErr() callback fired.
      queue_.timeOutSendingRequest(*this);
      return std::move(responsePayload_);

    case State::WRITE_SENT:
      // writeSuccess() or writeErr() processed this request but a response was
      // not received within the request's allotted timeout. Terminate request
      // with timeout.
      queue_.abortSentRequest(
          *this, TTransportException(TTransportException::TIMED_OUT));
      return std::move(responsePayload_);

    case State::COMPLETE:
      return std::move(responsePayload_);

    case State::DEFERRED_INIT:
    case State::WRITE_NOT_SCHEDULED:
    case State::WRITE_SCHEDULED:
      LOG(FATAL) << fmt::format(
          "Returned from Baton::wait() with unexpected state {} in {}",
          static_cast<int>(state_),
          __func__);
  }

  folly::assume_unreachable();
}

void RequestContext::onPayloadFrame(PayloadFrame&& payloadFrame) {
  DCHECK(!responsePayload_.hasException());
  DCHECK(isRequestResponse());

  if (LIKELY(!responsePayload_.hasValue())) {
    responsePayload_.emplace(std::move(payloadFrame.payload()));
  } else {
    responsePayload_->append(std::move(payloadFrame.payload()));
  }

  if (!payloadFrame.hasFollows()) {
    queue_.markAsResponded(*this);
  }
}

void RequestContext::onErrorFrame(ErrorFrame&& errorFrame) {
  DCHECK(!responsePayload_.hasException());
  DCHECK(isRequestResponse());

  responsePayload_ =
      folly::Try<Payload>(folly::make_exception_wrapper<RocketException>(
          errorFrame.errorCode(), std::move(errorFrame.payload()).data()));

  queue_.markAsResponded(*this);
}

void RequestContext::onWriteSuccess() noexcept {
  if (writeSuccessCallback_) {
    writeSuccessCallback_->onWriteSuccess();
  }
}

} // namespace apache::thrift::rocket
