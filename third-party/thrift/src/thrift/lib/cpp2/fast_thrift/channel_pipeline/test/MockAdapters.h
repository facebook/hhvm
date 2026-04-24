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

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

#include <functional>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

/**
 * MockHeadHandler satisfies HeadEndpointHandler concept.
 * Generic head endpoint mock for pipeline testing.
 * Records all writes and allows configuring responses.
 */
class MockHeadHandler {
 public:
  using WriteCallback = std::function<Result(BytesPtr)>;
  using OnWriteCallback = std::function<Result(TypeErasedBox&&)>;
  using OnExceptionCallback = std::function<void(folly::exception_wrapper&&)>;

  MockHeadHandler() = default;

  // HeadEndpointHandler data method
  Result onWrite(TypeErasedBox&& msg) noexcept {
    writeCount_++;
    if (onWriteCallback_) {
      return onWriteCallback_(std::move(msg));
    }
    auto bytes = std::move(msg.get<BytesPtr>());
    writtenBytes_.push_back(std::move(bytes));
    if (writeCallback_) {
      return writeCallback_(writtenBytes_.back()->clone());
    }
    return writeResult_;
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    exceptionCount_++;
    if (onExceptionCallback_) {
      onExceptionCallback_(std::move(e));
    }
  }

  // EndpointHandlerLifecycle methods
  void handlerAdded() noexcept { handlerAddedCount_++; }
  void handlerRemoved() noexcept { handlerRemovedCount_++; }
  void onPipelineActive() noexcept { pipelineActiveCount_++; }
  void onPipelineInactive() noexcept { pipelineInactiveCount_++; }

  // HeadEndpointHandler ready notification
  void onReadReady() noexcept { onReadReadyCount_++; }

  // Configuration
  void setWriteResult(Result result) { writeResult_ = result; }
  void setWriteCallback(WriteCallback cb) { writeCallback_ = std::move(cb); }
  void setOnWriteCallback(OnWriteCallback cb) {
    onWriteCallback_ = std::move(cb);
  }

  void setOnExceptionCallback(OnExceptionCallback cb) {
    onExceptionCallback_ = std::move(cb);
  }

  // Inspection
  int writeCount() const { return writeCount_; }
  int exceptionCount() const { return exceptionCount_; }

  const std::vector<BytesPtr>& writtenBytes() const { return writtenBytes_; }

  int handlerAddedCount() const { return handlerAddedCount_; }
  int handlerRemovedCount() const { return handlerRemovedCount_; }
  int pipelineActiveCount() const { return pipelineActiveCount_; }
  int pipelineInactiveCount() const { return pipelineInactiveCount_; }
  int onReadReadyCount() const { return onReadReadyCount_; }

  void reset() {
    writeCount_ = 0;
    exceptionCount_ = 0;
    writtenBytes_.clear();
    writeResult_ = Result::Success;
    writeCallback_ = nullptr;
    onWriteCallback_ = nullptr;
    onExceptionCallback_ = nullptr;
    handlerAddedCount_ = 0;
    handlerRemovedCount_ = 0;
    pipelineActiveCount_ = 0;
    pipelineInactiveCount_ = 0;
    onReadReadyCount_ = 0;
  }

 private:
  int writeCount_{0};
  int exceptionCount_{0};
  std::vector<BytesPtr> writtenBytes_;
  Result writeResult_{Result::Success};
  WriteCallback writeCallback_;
  OnWriteCallback onWriteCallback_;
  OnExceptionCallback onExceptionCallback_;
  int handlerAddedCount_{0};
  int handlerRemovedCount_{0};
  int pipelineActiveCount_{0};
  int pipelineInactiveCount_{0};
  int onReadReadyCount_{0};
};

/**
 * MockTailHandler satisfies TailEndpointHandler concept.
 * Generic tail endpoint mock for pipeline testing.
 * Records all messages received and allows configuring responses.
 */
class MockTailHandler {
 public:
  using OnReadCallback = std::function<Result(TypeErasedBox&&)>;
  using OnExceptionCallback = std::function<void(folly::exception_wrapper&&)>;

  MockTailHandler() = default;

  // TailEndpointHandler data methods
  Result onRead(TypeErasedBox&& msg) noexcept {
    readCount_++;
    if (onReadCallback_) {
      return onReadCallback_(std::move(msg));
    }
    return readResult_;
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    exceptionCount_++;
    if (onExceptionCallback_) {
      onExceptionCallback_(std::move(e));
    }
  }

  // EndpointHandlerLifecycle methods
  void handlerAdded() noexcept { handlerAddedCount_++; }
  void handlerRemoved() noexcept { handlerRemovedCount_++; }
  void onPipelineActive() noexcept { pipelineActiveCount_++; }
  void onPipelineInactive() noexcept { pipelineInactiveCount_++; }

  // TailEndpointHandler ready notification
  void onWriteReady() noexcept { onWriteReadyCount_++; }

  // Configuration
  void setReadResult(Result result) { readResult_ = result; }

  void setOnReadCallback(OnReadCallback cb) { onReadCallback_ = std::move(cb); }

  void setOnExceptionCallback(OnExceptionCallback cb) {
    onExceptionCallback_ = std::move(cb);
  }

  // Inspection
  int readCount() const { return readCount_; }
  int exceptionCount() const { return exceptionCount_; }

  int handlerAddedCount() const { return handlerAddedCount_; }
  int handlerRemovedCount() const { return handlerRemovedCount_; }
  int pipelineActiveCount() const { return pipelineActiveCount_; }
  int pipelineInactiveCount() const { return pipelineInactiveCount_; }
  int onWriteReadyCount() const { return onWriteReadyCount_; }

  void reset() {
    readCount_ = 0;
    exceptionCount_ = 0;
    readResult_ = Result::Success;
    onReadCallback_ = nullptr;
    onExceptionCallback_ = nullptr;
    handlerAddedCount_ = 0;
    handlerRemovedCount_ = 0;
    pipelineActiveCount_ = 0;
    pipelineInactiveCount_ = 0;
    onWriteReadyCount_ = 0;
  }

 private:
  int readCount_{0};
  int exceptionCount_{0};
  Result readResult_{Result::Success};
  OnReadCallback onReadCallback_;
  OnExceptionCallback onExceptionCallback_;
  int handlerAddedCount_{0};
  int handlerRemovedCount_{0};
  int pipelineActiveCount_{0};
  int pipelineInactiveCount_{0};
  int onWriteReadyCount_{0};
};

/**
 * TestAllocator is a simple allocator for testing.
 * Tracks allocations for verification.
 */
class TestAllocator {
 public:
  BytesPtr allocate(size_t size) {
    allocationCount_++;
    totalBytesAllocated_ += size;
    return folly::IOBuf::create(size);
  }

  BytesPtr copyBuffer(const void* data, size_t size) {
    allocationCount_++;
    totalBytesAllocated_ += size;
    return folly::IOBuf::copyBuffer(data, size);
  }

  int allocationCount() const { return allocationCount_; }

  size_t totalBytesAllocated() const { return totalBytesAllocated_; }

  void reset() {
    allocationCount_ = 0;
    totalBytesAllocated_ = 0;
  }

 private:
  int allocationCount_{0};
  size_t totalBytesAllocated_{0};
};

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
