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

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <utility>

#include <boost/intrusive/unordered_set.hpp>

#include <folly/IntrusiveList.h>
#include <folly/Likely.h>
#include <folly/Portability.h>
#include <folly/Traits.h>
#include <folly/fibers/Baton.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {
class RocketClient;
class RequestContextQueue;

class RequestContext {
 private:
  template <typename T>
  using payload_method_t = decltype(std::declval<const T&>().payload());

 public:
  class WriteSuccessCallback {
   public:
    virtual ~WriteSuccessCallback() = default;
    virtual void onWriteSuccess() noexcept = 0;
  };

  enum class State : uint8_t {
    DEFERRED_INIT, /* still needs to be intialized with server version */
    WRITE_NOT_SCHEDULED,
    WRITE_SCHEDULED,
    WRITE_SENDING, /* AsyncSocket::writeChain() called, but WriteCallback has
                      not yet fired */
    WRITE_SENT, /* Write to socket completed (possibly with error) */
    COMPLETE, /* Terminal state. Result stored in responsePayload_ */
  };

  template <class Frame>
  RequestContext(
      Frame&& frame,
      RequestContextQueue& queue,
      SetupFrame* setupFrame = nullptr,
      WriteSuccessCallback* writeSuccessCallback = nullptr,
      folly::IOBufFactory* ioBufFactory = nullptr)
      : queue_(queue),
        streamId_(frame.streamId()),
        frameType_(Frame::frameType()),
        writeSuccessCallback_(writeSuccessCallback),
        ioBufFactory_(ioBufFactory) {
    // Some `Frame`s lack a `payload()` method -- `RequestNFrame`,
    // `CancelFrame`, etc -- but those that do should have `.fds`.
    if constexpr (folly::is_detected<payload_method_t, Frame>::value) {
      fds = std::move(frame.payload().fds.dcheckToSendOrEmpty());
    }
    serialize(std::forward<Frame>(frame), setupFrame);
  }

  template <class InitFunc>
  RequestContext(
      InitFunc&& initFunc,
      int32_t serverVersion,
      StreamId streamId,
      RequestContextQueue& queue,
      WriteSuccessCallback* writeSuccessCallback = nullptr)
      : queue_(queue),
        streamId_(streamId),
        writeSuccessCallback_(writeSuccessCallback) {
    if (UNLIKELY(serverVersion == -1)) {
      deferredInit_ = std::forward<InitFunc>(initFunc);
      state_ = State::DEFERRED_INIT;
    } else {
      std::tie(serializedFrame_, frameType_) = initFunc(serverVersion);
    }
  }

  RequestContext(const RequestContext&) = delete;
  RequestContext(RequestContext&&) = delete;
  RequestContext& operator=(const RequestContext&) = delete;
  RequestContext& operator=(RequestContext&&) = delete;

  // For REQUEST_RESPONSE contexts, where an immediate matching response is
  // expected
  [[nodiscard]] folly::Try<Payload> waitForResponse(
      std::chrono::milliseconds timeout);
  [[nodiscard]] folly::Try<Payload> getResponse() &&;

  // For request types for which an immediate matching response is not
  // necessarily expected, e.g., REQUEST_FNF and REQUEST_STREAM
  [[nodiscard]] folly::Try<void> waitForWriteToComplete();

  void waitForWriteToCompleteSchedule(folly::fibers::Baton::Waiter* waiter);
  [[nodiscard]] folly::Try<void> waitForWriteToCompleteResult();

  void setTimeoutInfo(
      folly::HHWheelTimer& timer,
      folly::HHWheelTimer::Callback& callback,
      std::chrono::milliseconds timeout) {
    timer_ = &timer;
    timeoutCallback_ = &callback;
    requestTimeout_ = timeout;
  }

  void scheduleTimeoutForResponse() {
    DCHECK(isRequestResponse());
    // In some edge cases, response may arrive before write to socket finishes.
    if (state_ != State::COMPLETE &&
        requestTimeout_ != std::chrono::milliseconds::zero()) {
      timer_->scheduleTimeout(timeoutCallback_, requestTimeout_);
    }
  }

  std::unique_ptr<folly::IOBuf> releaseSerializedChain() {
    DCHECK(serializedFrame_);
    return std::move(serializedFrame_);
  }

  size_t endOffsetInBatch() const {
    DCHECK_GT(endOffsetInBatch_, 0);
    return endOffsetInBatch_;
  }

  void setEndOffsetInBatch(ssize_t offset) { endOffsetInBatch_ = offset; }

  State state() const { return state_; }

  StreamId streamId() const { return streamId_; }

  bool isRequestResponse() const {
    return frameType_ == FrameType::REQUEST_RESPONSE;
  }

  void onPayloadFrame(PayloadFrame&& payloadFrame);
  void onErrorFrame(ErrorFrame&& errorFrame);

  void onWriteSuccess() noexcept;

  bool hasPartialPayload() const { return responsePayload_.hasValue(); }

  void initWithVersion(int32_t serverVersion) {
    if (!deferredInit_) {
      return;
    }
    DCHECK(state_ == State::DEFERRED_INIT);
    std::tie(serializedFrame_, frameType_) = deferredInit_(serverVersion);
    DCHECK(serializedFrame_ && frameType_ != FrameType::RESERVED);
    state_ = State::WRITE_NOT_SCHEDULED;
  }

  folly::SocketFds fds;

 protected:
  friend class RocketClient;

  void markLastInWriteBatch() { lastInWriteBatch_ = true; }

 private:
  RequestContextQueue& queue_;
  folly::SafeIntrusiveListHook queueHook_;
  std::unique_ptr<folly::IOBuf> serializedFrame_;
  ssize_t endOffsetInBatch_{};
  StreamId streamId_;
  FrameType frameType_;
  State state_{State::WRITE_NOT_SCHEDULED};
  bool lastInWriteBatch_{false};
  bool isDummyEndOfBatchMarker_{false};

  boost::intrusive::unordered_set_member_hook<> setHook_;
  folly::fibers::Baton baton_;
  std::chrono::milliseconds requestTimeout_{1000};
  folly::HHWheelTimer* timer_{nullptr};
  folly::HHWheelTimer::Callback* timeoutCallback_{nullptr};
  folly::Try<Payload> responsePayload_;
  WriteSuccessCallback* const writeSuccessCallback_{nullptr};
  folly::IOBufFactory* ioBufFactory_{nullptr};
  folly::Function<std::pair<std::unique_ptr<folly::IOBuf>, FrameType>(int32_t)>
      deferredInit_{nullptr};

  template <class Frame>
  void serialize(Frame&& frame, SetupFrame* setupFrame) {
    DCHECK(!serializedFrame_);

    serializedFrame_ = std::move(frame).serialize(ioBufFactory_);

    if (UNLIKELY(setupFrame != nullptr)) {
      Serializer writer(ioBufFactory_);
      std::move(*setupFrame).serialize(writer);
      auto setupBuffer = std::move(writer).move();
      setupBuffer->prependChain(std::move(serializedFrame_));
      serializedFrame_ = std::move(setupBuffer);
    }
  }

  explicit RequestContext(RequestContextQueue& queue)
      : queue_(queue), frameType_(FrameType::REQUEST_RESPONSE) {}

  static RequestContext& createDummyEndOfBatchMarker(
      RequestContextQueue& queue) {
    auto* rctx = new RequestContext(queue);
    rctx->lastInWriteBatch_ = true;
    rctx->isDummyEndOfBatchMarker_ = true;
    rctx->state_ = State::WRITE_SENDING;
    return *rctx;
  }

  struct Equal {
    bool operator()(
        const RequestContext& ctxa, const RequestContext& ctxb) const noexcept {
      return ctxa.streamId_ == ctxb.streamId_;
    }
  };

  struct Hash {
    size_t operator()(const RequestContext& ctx) const noexcept {
      return std::hash<StreamId::underlying_type>()(
          static_cast<uint32_t>(ctx.streamId_));
    }
  };

 public:
  using Queue =
      folly::CountedIntrusiveList<RequestContext, &RequestContext::queueHook_>;

  using UnorderedSet = boost::intrusive::unordered_set<
      RequestContext,
      boost::intrusive::member_hook<
          RequestContext,
          decltype(setHook_),
          &RequestContext::setHook_>,
      boost::intrusive::equal<Equal>,
      boost::intrusive::hash<Hash>>;

 private:
  friend class RequestContextQueue;
};

} // namespace apache::thrift::rocket
