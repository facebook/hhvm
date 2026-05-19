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

#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

#include <fmt/core.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

namespace apache::thrift::fast_thrift::transport {

TransportHandler::TransportHandler(
    folly::AsyncTransport::UniquePtr socket,
    size_t minBufferSize,
    size_t maxBufferSize,
    std::chrono::milliseconds drainTimeout)
    : socket_(std::move(socket)),
      minBufferSize_(minBufferSize),
      maxBufferSize_(maxBufferSize),
      drainTimeoutDuration_(drainTimeout),
      socketDrainer_(this) {}

TransportHandler::~TransportHandler() {
  // Defensive: drop closeCallback_ before any teardown can fire it.
  // Caller-driven destruction without prior close() would otherwise
  // re-trigger destroy() via removeConnection and double-delete.
  closeCallback_ = nullptr;
  // Force a non-graceful close from Open so we reach Closed before
  // detaching the pipeline.
  if (state_ == State::Open) {
    closeImmediately(folly::exception_wrapper());
  }
  resetPipeline();
}

void TransportHandler::setPipeline(
    apache::thrift::fast_thrift::channel_pipeline::PipelineImpl*
        pipeline) noexcept {
  DCHECK(pipeline);
  DCHECK(state_ == State::Created);
  pipeline_ = pipeline;
  pipelineGuard_ =
      std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline_);
  state_ = State::Ready;
}

void TransportHandler::onConnect() noexcept {
  DCHECK(state_ == State::Ready);
  DCHECK(socket_->good());
  state_ = State::Open;
  resumeRead();
  pipeline_->activate();
}

void TransportHandler::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  auto tail = readBufQueue_.tailroom();
  if (tail < minBufferSize_) {
    const auto ret = readBufQueue_.preallocate(minBufferSize_, maxBufferSize_);
    *bufReturn = ret.first;
    *lenReturn = ret.second;
  } else {
    *bufReturn = readBufQueue_.writableTail();
    *lenReturn = tail;
  }
}

void TransportHandler::handleReadResult(Result result) noexcept {
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

void TransportHandler::readDataAvailable(size_t len) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  readBufQueue_.postallocate(len);
  DCHECK(pipeline_);
  handleReadResult(pipeline_->fireRead(TypeErasedBox(readBufQueue_.move())));
}

void TransportHandler::readBufferAvailable(
    std::unique_ptr<folly::IOBuf> buf) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  DCHECK(pipeline_);
  handleReadResult(pipeline_->fireRead(TypeErasedBox(std::move(buf))));
}

void TransportHandler::readEOF() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  closeGracefully(
      apache::thrift::transport::TTransportException(
          apache::thrift::transport::TTransportException::
              TTransportExceptionType::END_OF_FILE,
          "Channel got EOF."));
}

void TransportHandler::readErr(const folly::AsyncSocketException& ex) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  closeGracefully(apache::thrift::transport::TTransportException(ex));
}

void TransportHandler::handlerRemoved() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  closeImmediately(folly::exception_wrapper());
}

TransportHandler::Result TransportHandler::onWrite(
    TypeErasedBox&& msg) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  auto bytes = std::move(msg.get<BytesPtr>());
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

void TransportHandler::pauseRead() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  if (FOLLY_UNLIKELY(readPaused_)) {
    return;
  }
  readPaused_ = true;
  socket_->setReadCB(nullptr);
}

void TransportHandler::resumeRead() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  if (FOLLY_UNLIKELY(!readPaused_)) {
    return;
  }
  readPaused_ = false;
  socket_->setReadCB(this);
}

void TransportHandler::close(folly::exception_wrapper&& ex) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  closeGracefully(std::move(ex));
}

void TransportHandler::resetPipeline() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  // Pipeline must not be actively dispatching when we drop it.
  DCHECK(state_ != State::Open);
  pipeline_ = nullptr;
  pipelineGuard_.reset();
}

void TransportHandler::maybeReleaseDrainer() noexcept {
  // Pre-connect writes (state == Ready) can land in writeSuccess too, but
  // they must not collapse the state machine into Closed.
  if (state_ == State::Closing && writePending_ == 0) {
    socketDrainer_.release();
  }
}

void TransportHandler::writeSuccess() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  --writePending_;
  if (state_ == State::Open) {
    DCHECK(pipeline_);
    pipeline_->onWriteReady();
    return;
  }
  maybeReleaseDrainer();
}

void TransportHandler::writeErr(
    size_t bytesWritten, const folly::AsyncSocketException& ex) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  --writePending_;
  closeGracefully(
      folly::AsyncSocketException(
          ex.getType(),
          fmt::format(
              "Write failed with bytesWritten={}: {}", bytesWritten, ex.what()),
          ex.getErrno()));
  maybeReleaseDrainer();
}

bool TransportHandler::beginClose(folly::exception_wrapper ex) noexcept {
  if (state_ != State::Open) {
    return false;
  }
  state_ = State::Closing;
  pauseRead();
  if (FOLLY_LIKELY(pipeline_)) {
    if (ex) {
      pipeline_->fireException(std::move(ex));
    }
    pipeline_->deactivate();
  }
  return true;
}

void TransportHandler::closeGracefully(folly::exception_wrapper ex) noexcept {
  if (!beginClose(std::move(ex))) {
    return;
  }
  if (writePending_ > 0) {
    socketDrainer_.start();
  } else {
    closeNow();
  }
}

void TransportHandler::closeImmediately(folly::exception_wrapper ex) noexcept {
  if (!beginClose(std::move(ex))) {
    return;
  }
  closeNow();
}

void TransportHandler::closeNow() noexcept {
  if (state_ == State::Closed) {
    return;
  }
  // Set state first so any re-entrant writeErrs from the cascade below
  // see a terminal state and don't re-enter beginClose.
  state_ = State::Closed;
  socket_->closeNow();
  if (auto closeCallback = std::move(closeCallback_)) {
    closeCallback();
  }
}

// --- SocketDrainer ---

TransportHandler::SocketDrainer::SocketDrainer(TransportHandler* self)
    : folly::AsyncTimeout(self->socket_->getEventBase()), self_(self) {}

void TransportHandler::SocketDrainer::start() {
  if (guard_.has_value()) {
    return;
  }
  guard_.emplace(self_);
  scheduleTimeout(self_->drainTimeoutDuration_);
}

void TransportHandler::SocketDrainer::release() {
  cancelTimeout();
  guard_.reset();
  self_->closeNow();
}

void TransportHandler::SocketDrainer::timeoutExpired() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(self_);
  if (self_->state_ == State::Closing && self_->writePending_ > 0) {
    XLOGF(
        WARNING,
        "TransportHandler drain timed out with writePending={}; force-closing socket",
        self_->writePending_);
  }
  release();
}

} // namespace apache::thrift::fast_thrift::transport
