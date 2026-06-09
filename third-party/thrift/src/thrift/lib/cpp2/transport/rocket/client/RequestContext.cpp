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

#include <chrono>

#include <glog/logging.h>

#include <fmt/core.h>
#include <folly/Likely.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Assume.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/RpcTransportStats.h>
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
    // Record the moment the first PAYLOAD frame received for this
    // response (used to decompose
    // RpcTransportStats::responseRoundTripLatency into a first-frame
    // interval and a post-first-frame tail). Set only on the first
    // frame received; not updated by subsequent fragments.
    firstResponsePayloadFrameTime_ = std::chrono::steady_clock::now();
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
  // Delegate to the channel callback FIRST so its timeEndSend_ is sampled
  // before the rocket-side sample below. This preserves the public
  // invariant firstResponsePayloadFrameLatency <= responseRoundTripLatency.
  if (writeSuccessCallback_) {
    writeSuccessCallback_->onWriteSuccess();
  }
  // Sample rocket-side timeEndSend_ AFTER the channel callback returns.
  // The two samples are nanoseconds apart on the same thread; the rocket
  // value is monotonically >= the channel value.
  timeEndSend_ = std::chrono::steady_clock::now();
}

void RequestContext::finalizeRpcTransportStats() noexcept {
  if (channelStats_ == nullptr) {
    return;
  }
  // Defensive: no payload frame was ever received (error/timeout paths
  // that still flow through markAsResponded). Leave the field at zero.
  if (firstResponsePayloadFrameTime_.time_since_epoch().count() == 0) {
    return;
  }
  // Clamp: in the RequestContextQueue::markAsResponded() else-branch the
  // synthesized onWriteSuccess() runs AFTER the first PAYLOAD frame has
  // already been observed, so firstResponsePayloadFrameTime_ can be less
  // than timeEndSend_. Leave the field at zero rather than reporting a
  // negative I/O latency.
  if (firstResponsePayloadFrameTime_ >= timeEndSend_) {
    channelStats_->firstResponsePayloadFrameLatency =
        firstResponsePayloadFrameTime_ - timeEndSend_;
  }
}

} // namespace apache::thrift::rocket
