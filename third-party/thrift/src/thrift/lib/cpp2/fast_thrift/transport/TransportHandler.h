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

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>

namespace apache::thrift::fast_thrift::transport {

/**
 * Bridges the channel_pipeline framework with an async socket.
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
class TransportHandler : public folly::DelayedDestruction,
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

  TransportHandler(const TransportHandler&) = delete;
  TransportHandler& operator=(const TransportHandler&) = delete;
  TransportHandler(TransportHandler&&) = delete;
  TransportHandler& operator=(TransportHandler&&) = delete;

  using Ptr =
      std::unique_ptr<TransportHandler, folly::DelayedDestruction::Destructor>;

  static Ptr create(
      folly::AsyncTransport::UniquePtr socket,
      size_t minBufferSize = 4096,
      size_t maxBufferSize = 65536,
      std::chrono::milliseconds drainTimeout = kDefaultDrainTimeout) {
    return Ptr(new TransportHandler(
        std::move(socket), minBufferSize, maxBufferSize, drainTimeout));
  }

  State state() const noexcept { return state_; }

  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl*
          pipeline) noexcept;

  void setCloseCallback(folly::Function<void()> closeCallback) noexcept {
    closeCallback_ = std::move(closeCallback);
  }

  bool setZeroCopy(bool enable) noexcept {
    return socket_->setZeroCopy(enable);
  }

  void setZeroCopyEnableThreshold(size_t threshold) noexcept {
    socket_->setZeroCopyEnableThreshold(threshold);
  }

  void onConnect() noexcept;

  // --- AsyncTransport::ReadCallback interface ---

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  void readDataAvailable(size_t len) noexcept override;
  void readBufferAvailable(std::unique_ptr<folly::IOBuf> buf) noexcept override;
  void readEOF() noexcept override;
  void readErr(const folly::AsyncSocketException& ex) noexcept override;
  bool isBufferMovable() noexcept override { return true; }

  // --- TailEndpointHandler lifecycle ---

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept;
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

  void onReadReady() noexcept { resumeRead(); }

  // --- TailEndpointHandler interface (OutboundTransportHandler refines) ---

  Result onWrite(TypeErasedBox&& msg) noexcept;

  void pauseRead() noexcept;
  void resumeRead() noexcept;

  // Graceful disconnect: pending writes drain naturally; drain timeout is
  // the safety net. Idempotent (no-op outside Open).
  void close(folly::exception_wrapper&& ex) noexcept;

  // Destruction helper: detaches the pipeline. Forces a non-graceful close
  // from Open so the queue is drained inline (no caller left to wait).
  // Not a public state-machine entry; ~T calls this.
  void resetPipeline() noexcept;

  // --- AsyncTransport::WriteCallback interface ---

  void writeSuccess() noexcept override;
  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept override;

  ~TransportHandler() override;

 protected:
  TransportHandler(
      folly::AsyncTransport::UniquePtr socket,
      size_t minBufferSize,
      size_t maxBufferSize,
      std::chrono::milliseconds drainTimeout);

 private:
  // Dispatch a pipeline read result: Success falls through, Backpressure
  // pauses reads, Error closes gracefully. Shared by readDataAvailable +
  // readBufferAvailable.
  void handleReadResult(Result result) noexcept;

  // If draining toward close and the last pending write just landed, drop
  // the drain guard and finish closing. Shared by writeSuccess + writeErr.
  void maybeReleaseDrainer() noexcept;

  // Open -> Closing prelude: state flip, pause reads, notify pipeline.
  // Returns false if state wasn't Open (idempotent no-op). Doesn't decide
  // how to reach Closed — callers compose with the drain/closeNow step.
  bool beginClose(folly::exception_wrapper ex) noexcept;

  // Graceful: let pending writes finish via writeSuccess; SocketDrainer is
  // the timeout safety net. No pending writes collapses straight to Closed.
  void closeGracefully(folly::exception_wrapper ex) noexcept;

  // Immediate: closeNow() inline cascades writeErrs synchronously and drains
  // writePending_ to 0 before returning.
  void closeImmediately(folly::exception_wrapper ex) noexcept;

  // Closing -> Closed. Idempotent (no-op outside Closing).
  void closeNow() noexcept;

  // Owns the drain DestructorGuard and the timeout. start() takes the guard
  // and schedules the timer; release() drops both. The timer firing forces a
  // closeNow() on the socket to cascade writeErrs and drain the queue, then
  // releases itself.
  class SocketDrainer : public folly::AsyncTimeout {
   public:
    explicit SocketDrainer(TransportHandler* self);
    void start();
    void release();
    bool active() const { return guard_.has_value(); }
    void timeoutExpired() noexcept override;

   private:
    TransportHandler* self_;
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
  folly::Function<void()> closeCallback_;
};

} // namespace apache::thrift::fast_thrift::transport
