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
#include <optional>
#include <type_traits>

#include <fmt/core.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::transport {

/**
 * Bridges the channel_pipeline framework with an async socket.
 *
 * Templated on a `WriteCompleteEventFactory`-satisfying type (see
 * WriteCompletion.h). The default `NoOpWriteCompleteEventFactory` causes
 * signalWriteComplete (the event hook in writeSuccess/writeErr) to be fully
 * elided via `if constexpr` (detected via `std::is_same_v`). Pipelines that
 * want per-write notifications instantiate with a real factory.
 *
 * State machine (one-directional; per-connection, not reused):
 *   Created   --setPipeline-->   Ready
 *   Ready     --onConnect-->     Open
 *   Open      --close (any)-->   Closing
 *   Closing   --drain done-->    Closed
 *
 * Disconnect signals (Open -> Closing): close(), readEOF, readErr, writeErr,
 * pipeline read error. The SocketDrainer takes a DestructorGuard while
 * draining so the handler outlives any WriteRequests still queued in
 * AsyncSocket. Closing -> Closed when writePending_ hits 0 (natural drain)
 * or the drain timeout fires (force closeNow cascades writeErrs).
 */
template <WriteCompleteEventFactory Factory = NoOpWriteCompleteEventFactory>
class TransportHandlerT : public folly::DelayedDestruction,
                          public folly::AsyncTransport::ReadCallback,
                          public folly::AsyncTransport::WriteCallback {
 public:
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  using Result = apache::thrift::fast_thrift::channel_pipeline::Result;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

  enum class State : uint8_t {
    Created,
    Ready,
    Open,
    Closing,
    Closed,
  };

  static constexpr std::chrono::milliseconds kDefaultDrainTimeout{30'000};

  TransportHandlerT(const TransportHandlerT&) = delete;
  TransportHandlerT& operator=(const TransportHandlerT&) = delete;
  TransportHandlerT(TransportHandlerT&&) = delete;
  TransportHandlerT& operator=(TransportHandlerT&&) = delete;

  using Ptr =
      std::unique_ptr<TransportHandlerT, folly::DelayedDestruction::Destructor>;

  static Ptr create(
      folly::AsyncTransport::UniquePtr socket,
      size_t minBufferSize = 4096,
      size_t maxBufferSize = 65536,
      std::chrono::milliseconds drainTimeout = kDefaultDrainTimeout) {
    return Ptr(new TransportHandlerT(
        std::move(socket), minBufferSize, maxBufferSize, drainTimeout));
  }

  State state() const noexcept { return state_; }

  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl*
          pipeline) noexcept {
    DCHECK(pipeline);
    DCHECK(state_ == State::Created);
    pipeline_ = pipeline;
    pipelineGuard_ =
        std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline_);
    state_ = State::Ready;
  }

  bool setZeroCopy(bool enable) noexcept {
    return socket_->setZeroCopy(enable);
  }

  void setZeroCopyEnableThreshold(size_t threshold) noexcept {
    socket_->setZeroCopyEnableThreshold(threshold);
  }

  void onConnect() noexcept {
    DCHECK(state_ == State::Ready);
    DCHECK(socket_->good());
    state_ = State::Open;
    resumeRead();
    pipeline_->activate();
  }

  // --- AsyncTransport::ReadCallback interface ---

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    auto tail = readBufQueue_.tailroom();
    if (tail < minBufferSize_) {
      const auto ret =
          readBufQueue_.preallocate(minBufferSize_, maxBufferSize_);
      *bufReturn = ret.first;
      *lenReturn = ret.second;
    } else {
      *bufReturn = readBufQueue_.writableTail();
      *lenReturn = tail;
    }
  }

  void readDataAvailable(size_t len) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    readBufQueue_.postallocate(len);
    DCHECK(pipeline_);
    handleReadResult(pipeline_->fireRead(TypeErasedBox(readBufQueue_.move())));
  }

  void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> buf) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    DCHECK(pipeline_);
    handleReadResult(pipeline_->fireRead(TypeErasedBox(std::move(buf))));
  }

  void readEOF() noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    closeGracefully(
        apache::thrift::transport::TTransportException(
            apache::thrift::transport::TTransportException::
                TTransportExceptionType::END_OF_FILE,
            "Channel got EOF."));
  }

  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    closeGracefully(apache::thrift::transport::TTransportException(ex));
  }

  bool isBufferMovable() noexcept override { return true; }

  // --- TailEndpointHandler lifecycle ---

  void handlerAdded() noexcept {}

  void handlerRemoved() noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    closeImmediately(folly::exception_wrapper());
  }

  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

  void onReadReady() noexcept { resumeRead(); }

  // --- TailEndpointHandler interface (OutboundTransportHandler refines) ---

  Result onWrite(TypeErasedBox&& msg) noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    auto bytes = std::move(msg.template get<BytesPtr>());
    DCHECK(bytes);
    // Accept writes from Ready (pre-connect) too — AsyncSocket buffers them
    // until the connect completes and then flushes. Without this, a client
    // that fires send() immediately after constructing the socket racing
    // against connectSuccess() silently drops the bytes. Only refuse in
    // terminal / pre-pipeline states.
    if (FOLLY_UNLIKELY(state_ != State::Open && state_ != State::Ready)) {
      return Result::Error;
    }

    ++writePending_;

    try {
      socket_->writeChain(this, std::move(bytes));
    } catch (...) {
      --writePending_;
      return Result::Error;
    }

    return writePending_ > 0 ? Result::Backpressure : Result::Success;
  }

  void pauseRead() noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    if (FOLLY_UNLIKELY(readPaused_)) {
      return;
    }
    readPaused_ = true;
    socket_->setReadCB(nullptr);
  }

  void resumeRead() noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    if (FOLLY_UNLIKELY(!readPaused_)) {
      return;
    }
    readPaused_ = false;
    socket_->setReadCB(this);
  }

  // Graceful disconnect: pending writes drain naturally; drain timeout is
  // the safety net. Idempotent (no-op outside Open).
  void close(folly::exception_wrapper&& ex) noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    closeGracefully(std::move(ex));
  }

  // Destruction helper: detaches the pipeline. Forces a non-graceful close
  // from Open so the queue is drained inline (no caller left to wait).
  // Not a public state-machine entry; ~T calls this.
  void resetPipeline() noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    // Pipeline must not be actively dispatching when we drop it.
    DCHECK(state_ != State::Open);
    pipeline_ = nullptr;
    pipelineGuard_.reset();
  }

  // --- AsyncTransport::WriteCallback interface ---

  void writeSuccess() noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    --writePending_;
    if (state_ == State::Open) {
      DCHECK(pipeline_);
      signalWriteComplete(WriteCompletionStatus::Success, 0);
      pipeline_->onWriteReady();
      return;
    }
    maybeReleaseDrainer();
  }

  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    --writePending_;
    signalWriteComplete(WriteCompletionStatus::Error, bytesWritten);
    closeGracefully(
        folly::AsyncSocketException(
            ex.getType(),
            fmt::format(
                "Write failed with bytesWritten={}: {}",
                bytesWritten,
                ex.what()),
            ex.getErrno()));
    maybeReleaseDrainer();
  }

  ~TransportHandlerT() override {
    // Force a non-graceful close from Open so we reach Closed before
    // detaching the pipeline.
    if (state_ == State::Open) {
      closeImmediately(folly::exception_wrapper());
    }
    resetPipeline();
  }

 protected:
  TransportHandlerT(
      folly::AsyncTransport::UniquePtr socket,
      size_t minBufferSize,
      size_t maxBufferSize,
      std::chrono::milliseconds drainTimeout)
      : socket_(std::move(socket)),
        minBufferSize_(minBufferSize),
        maxBufferSize_(maxBufferSize),
        drainTimeoutDuration_(drainTimeout),
        socketDrainer_(this) {}

 private:
  // Build and fire the per-write completion event through Factory. Elided
  // entirely for the NoOp factory. Completion events fire only while Open:
  // once a close begins (Closing/Closed) both success and error completions
  // are suppressed symmetrically, so consumers must treat pipeline
  // deactivation as abandonment of any writes still in flight. The gate also
  // covers the destructor-triggered close cascade (writeErrs landing after
  // the pipeline detached). Open guarantees pipeline_ is attached. Shared by
  // writeSuccess + writeErr.
  void signalWriteComplete(
      WriteCompletionStatus status, size_t bytes) noexcept {
    if constexpr (!std::is_same_v<Factory, NoOpWriteCompleteEventFactory>) {
      if (FOLLY_LIKELY(state_ == State::Open)) {
        DCHECK(pipeline_);
        auto [eventId, eventMsg] = Factory::make(status, bytes);
        pipeline_->fireEvent(eventId, std::move(eventMsg));
      }
    }
  }

  // Dispatch a pipeline read result: Success falls through, Backpressure
  // pauses reads, Error closes gracefully. Shared by readDataAvailable +
  // readBufferAvailable.
  void handleReadResult(Result result) noexcept {
    if (FOLLY_LIKELY(result == Result::Success)) {
      return;
    }
    if (result == Result::Backpressure) {
      pauseRead();
    } else {
      DCHECK(result == Result::Error);
      closeGracefully(
          folly::AsyncSocketException(
              folly::AsyncSocketException::UNKNOWN, "Pipeline read error"));
    }
  }

  // If draining toward close and the last pending write just landed, drop
  // the drain guard and finish closing. Shared by writeSuccess + writeErr.
  void maybeReleaseDrainer() noexcept {
    // Pre-connect writes (state == Ready) can land in writeSuccess too, but
    // they must not collapse the state machine into Closed.
    if (state_ == State::Closing && writePending_ == 0) {
      socketDrainer_.release();
    }
  }

  // Open -> Closing prelude: state flip, pause reads, notify pipeline.
  // Returns false if state wasn't Open (idempotent no-op). Doesn't decide
  // how to reach Closed — callers compose with the drain/closeNow step.
  bool beginClose(folly::exception_wrapper ex) noexcept {
    if (state_ != State::Open) {
      return false;
    }
    state_ = State::Closing;
    pauseRead();
    if (pipeline_) {
      if (ex) {
        pipeline_->fireException(std::move(ex));
      }
      pipeline_->deactivate();
    }
    return true;
  }

  // Graceful: let pending writes finish via writeSuccess; SocketDrainer is
  // the timeout safety net. No pending writes collapses straight to Closed.
  void closeGracefully(folly::exception_wrapper ex) noexcept {
    if (!beginClose(std::move(ex))) {
      return;
    }
    if (writePending_ > 0) {
      socketDrainer_.start();
    } else {
      closeNow();
    }
  }

  // Immediate: closeNow() inline cascades writeErrs synchronously and drains
  // writePending_ to 0 before returning.
  void closeImmediately(folly::exception_wrapper ex) noexcept {
    if (!beginClose(std::move(ex))) {
      return;
    }
    closeNow();
  }

  // Closing -> Closed. Idempotent (no-op outside Closing).
  void closeNow() noexcept {
    if (state_ == State::Closed) {
      return;
    }
    // Set state first so any re-entrant writeErrs from the cascade below
    // see a terminal state and don't re-enter beginClose.
    state_ = State::Closed;
    socket_->closeNow();
  }

  // Owns the drain DestructorGuard and the timeout. start() takes the guard
  // and schedules the timer; release() drops both. The timer firing forces a
  // closeNow() on the socket to cascade writeErrs and drain the queue, then
  // releases itself.
  class SocketDrainer : public folly::AsyncTimeout {
   public:
    explicit SocketDrainer(TransportHandlerT* self)
        : folly::AsyncTimeout(self->socket_->getEventBase()), self_(self) {}

    void start() {
      if (guard_.has_value()) {
        return;
      }
      guard_.emplace(self_);
      scheduleTimeout(self_->drainTimeoutDuration_);
    }

    void release() {
      cancelTimeout();
      guard_.reset();
      self_->closeNow();
    }

    bool active() const { return guard_.has_value(); }

    void timeoutExpired() noexcept override {
      folly::DelayedDestruction::DestructorGuard dg(self_);
      if (self_->state_ == State::Closing && self_->writePending_ > 0) {
        XLOGF(
            WARNING,
            "TransportHandler drain timed out with writePending={}; force-closing socket",
            self_->writePending_);
      }
      release();
    }

   private:
    TransportHandlerT* self_;
    std::optional<folly::DelayedDestruction::DestructorGuard> guard_;
  };

  folly::AsyncTransport::UniquePtr socket_;
  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
  size_t minBufferSize_;
  size_t maxBufferSize_;
  std::chrono::milliseconds drainTimeoutDuration_;
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  State state_{State::Created};
  bool readPaused_{true};
  uint32_t writePending_{0};
  SocketDrainer socketDrainer_;
};

// Default specialization — preserves the existing TransportHandler name for
// callers that don't need write-completion events. New consumers
// (rocket-client/server pipelines) instantiate TransportHandlerT<RealFactory>
// directly.
using TransportHandler = TransportHandlerT<NoOpWriteCompleteEventFactory>;

} // namespace apache::thrift::fast_thrift::transport
