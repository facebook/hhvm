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

#include <fmt/core.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>

namespace apache::thrift::fast_thrift::transport {

/**
 * TransportHandler bridges the channel_pipeline framework with the underlying
 * async transport (socket).
 *
 * This handler implements:
 *
 * - AsyncTransport::ReadCallback: Receives data from the socket and fires
 *   raw bytes to the pipeline via fireRead().
 *
 * - AsyncTransport::WriteCallback: Handles async write completion and
 *   propagates write ready notifications to the pipeline.
 *
 * - OutboundTransportHandler concept: Receives bytes from the pipeline and
 *   writes them to the underlying transport.
 *
 * Only one write can be pending at a time; additional writes return
 * Backpressure until the pending write completes.
 *
 * Extends folly::DelayedDestruction to ensure safe destruction during
 * callbacks, similar to RocketClient.
 */
class TransportHandler : public folly::DelayedDestruction,
                         public folly::AsyncTransport::ReadCallback,
                         public folly::AsyncTransport::WriteCallback {
 public:
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  using Result = apache::thrift::fast_thrift::channel_pipeline::Result;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

  TransportHandler(const TransportHandler&) = delete;
  TransportHandler& operator=(const TransportHandler&) = delete;
  TransportHandler(TransportHandler&&) = delete;
  TransportHandler& operator=(TransportHandler&&) = delete;

  using Ptr =
      std::unique_ptr<TransportHandler, folly::DelayedDestruction::Destructor>;

  static Ptr create(
      folly::AsyncTransport::UniquePtr socket,
      size_t minBufferSize = 4096,
      size_t maxBufferSize = 65536) {
    return Ptr(
        new TransportHandler(std::move(socket), minBufferSize, maxBufferSize));
  }

  /**
   * Phase 2: Set the pipeline reference.
   * Must be called before any read/write operations.
   * Stores a DestructorGuard to ensure the pipeline outlives the transport.
   */
  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl&
          pipeline) noexcept {
    if (pipeline_) {
      XLOG(FATAL) << "must reset pipeline before setting a new one";
    }

    pipelineGuard_ =
        std::make_unique<folly::DelayedDestruction::DestructorGuard>(&pipeline);
    pipeline_ = &pipeline;

    // If the socket is already connected, fire the connect event.
    if (socket_->good()) {
      onConnect();
    }
  }

  /**
   * Set the close callback to be invoked when the transport closes.
   * This allows the connection manager to be notified of connection closure.
   */
  void setCloseCallback(folly::Function<void()> closeCallback) noexcept {
    closeCallback_ = std::move(closeCallback);
  }

  /**
   * Enable MSG_ZEROCOPY on the underlying socket.
   * Sets the SO_ZEROCOPY sockopt. Returns false if not supported.
   */
  bool setZeroCopy(bool enable) noexcept {
    return socket_->setZeroCopy(enable);
  }

  /**
   * Set the minimum payload size for automatic MSG_ZEROCOPY.
   * Writes smaller than this threshold use normal sendmsg().
   */
  void setZeroCopyEnableThreshold(size_t threshold) noexcept {
    socket_->setZeroCopyEnableThreshold(threshold);
  }

  void onConnect() noexcept {
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
    auto buf = readBufQueue_.move();
    auto msg = TypeErasedBox(std::move(buf));
    Result result = pipeline_->fireRead(std::move(msg));

    switch (result) {
      case Result::Success:
        break;
      case Result::Backpressure:
        pauseRead();
        break;
      case Result::Error:
        closeInternal(
            folly::AsyncSocketException(
                folly::AsyncSocketException::UNKNOWN, "Pipeline read error"));
        break;
    }
  }

  void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> buf) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);

    auto msg = TypeErasedBox(std::move(buf));
    Result result = pipeline_->fireRead(std::move(msg));

    switch (result) {
      case Result::Success:
        break;
      case Result::Backpressure:
        pauseRead();
        break;
      case Result::Error:
        closeInternal(
            folly::AsyncSocketException(
                folly::AsyncSocketException::UNKNOWN, "Pipeline read error"));
        break;
    }
  }

  void readEOF() noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);

    closeInternal(
        apache::thrift::transport::TTransportException(
            apache::thrift::transport::TTransportException::
                TTransportExceptionType::END_OF_FILE,
            "Channel got EOF. Check for server hitting connection limit, "
            "connection age timeout, server connection idle timeout, and server "
            "crashes."));
  }

  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    closeInternal(apache::thrift::transport::TTransportException(ex));
  }

  bool isBufferMovable() noexcept override { return true; }

  // --- TailEndpointHandler lifecycle ---

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

  /**
   * Called by the pipeline when downstream backpressure releases and reads
   * can resume. Resumes reads on the underlying socket.
   */
  void onReadReady() noexcept { resumeRead(); }

  // --- TailEndpointHandler interface (OutboundTransportHandler refines) ---

  /**
   * Write a message to the transport.
   * Called by the pipeline to push data toward the socket.
   *
   * The TypeErasedBox must contain a BytesPtr. The bytes are extracted and
   * written to the underlying socket.
   *
   * Only one write can be pending at a time. If a write is already pending,
   * returns Result::Backpressure. The caller should retry after the pending
   * write completes (signaled via writeSuccess()).
   */
  Result onWrite(TypeErasedBox&& msg) noexcept {
    auto bytes = std::move(msg.get<BytesPtr>());
    DCHECK(bytes);
    folly::DelayedDestruction::DestructorGuard dg(this);
    if (closed_) {
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

  /**
   * Pause reading from the transport (backpressure).
   */
  void pauseRead() noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    if (readPaused_) {
      return;
    }
    readPaused_ = true;
    socket_->setReadCB(nullptr);
  }

  /**
   * Resume reading from the transport.
   */
  void resumeRead() noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    if (!readPaused_) {
      return;
    }
    readPaused_ = false;
    socket_->setReadCB(this);
  }

  /**
   * Called when the transport is closed externally.
   * Triggers internal close logic and notifies the close callback.
   */
  void onClose(folly::exception_wrapper&& ex) noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    closeInternal(std::move(ex));
  }

  void onException(folly::exception_wrapper&& ex) noexcept {
    folly::DelayedDestruction::DestructorGuard dg(this);
    closeInternal(std::move(ex));
  }

  // --- AsyncTransport::WriteCallback interface ---

  void writeSuccess() noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    --writePending_;
    pipeline_->onWriteReady();
  }

  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    --writePending_;
    auto newEx = folly::AsyncSocketException(
        ex.getType(),
        fmt::format(
            "Write failed with bytesWritten={}: {}", bytesWritten, ex.what()),
        ex.getErrno());
    closeInternal(newEx);
  }

  ~TransportHandler() override { closeInternal(folly::exception_wrapper()); }

 protected:
  TransportHandler(
      folly::AsyncTransport::UniquePtr socket,
      size_t minBufferSize,
      size_t maxBufferSize)
      : socket_(std::move(socket)),
        minBufferSize_(minBufferSize),
        maxBufferSize_(maxBufferSize) {}

 private:
  void resetPipeline() noexcept {
    pipeline_ = nullptr;
    pipelineGuard_.reset();
  }

  void closeInternal(folly::exception_wrapper ex) noexcept {
    if (closed_) {
      return;
    }

    closed_ = true;

    pauseRead();
    socket_->closeNow();

    if (pipeline_) {
      if (ex) {
        (void)pipeline_->fireExceptionFromIndex(0, std::move(ex));
      }
      pipeline_->deactivate();
    }

    resetPipeline();

    if (auto closeCallback = std::move(closeCallback_)) {
      closeCallback();
    }
  }

  folly::AsyncTransport::UniquePtr socket_;
  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
  size_t minBufferSize_;
  size_t maxBufferSize_;
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  bool readPaused_{true};
  uint32_t writePending_{0};
  bool closed_{false};
  folly::Function<void()> closeCallback_;
};

} // namespace apache::thrift::fast_thrift::transport
