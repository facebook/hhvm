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
  using OnReadCallback = std::function<Result(TypeErasedBox&&)>;
  using OnExceptionCallback = std::function<void(folly::exception_wrapper&&)>;

  MockHeadHandler() = default;

  // HeadEndpointHandler data method
  Result onWrite(TypeErasedBox&& msg) noexcept {
    writeCount_++;
    auto bytes = std::move(msg.get<BytesPtr>());
    writtenBytes_.push_back(std::move(bytes));
    if (writeCallback_) {
      return writeCallback_(writtenBytes_.back()->clone());
    }
    return writeResult_;
  }

  // Legacy EndpointHandler methods (for backward compatibility)
  Result onMessage(TypeErasedBox&& msg) noexcept {
    return onWrite(std::move(msg));
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

  // Configuration
  void setWriteResult(Result result) { writeResult_ = result; }
  void setWriteCallback(WriteCallback cb) { writeCallback_ = std::move(cb); }

  // Legacy aliases - TODO: Remove after tests are migrated
  void setMessageResult(Result result) { writeResult_ = result; }
  void setOnReadCallback(OnReadCallback cb) {
    writeCallback_ = [cb = std::move(cb)](BytesPtr bytes) {
      return cb(TypeErasedBox(std::move(bytes)));
    };
  }
  void setOnMessageCallback(OnReadCallback cb) {
    setOnReadCallback(std::move(cb));
  }
  void setOnExceptionCallback(OnExceptionCallback cb) {
    onExceptionCallback_ = std::move(cb);
  }

  // Inspection
  int writeCount() const { return writeCount_; }
  int messageCount() const { return writeCount_; } // TODO: Remove
  int exceptionCount() const { return exceptionCount_; }

  const std::vector<BytesPtr>& writtenBytes() const { return writtenBytes_; }

  int handlerAddedCount() const { return handlerAddedCount_; }
  int handlerRemovedCount() const { return handlerRemovedCount_; }
  int pipelineActiveCount() const { return pipelineActiveCount_; }
  int pipelineInactiveCount() const { return pipelineInactiveCount_; }

  void reset() {
    writeCount_ = 0;
    exceptionCount_ = 0;
    writtenBytes_.clear();
    writeResult_ = Result::Success;
    writeCallback_ = nullptr;
    onExceptionCallback_ = nullptr;
    handlerAddedCount_ = 0;
    handlerRemovedCount_ = 0;
    pipelineActiveCount_ = 0;
    pipelineInactiveCount_ = 0;
  }

 private:
  int writeCount_{0};
  int exceptionCount_{0};
  std::vector<BytesPtr> writtenBytes_;
  Result writeResult_{Result::Success};
  WriteCallback writeCallback_;
  OnExceptionCallback onExceptionCallback_;
  int handlerAddedCount_{0};
  int handlerRemovedCount_{0};
  int pipelineActiveCount_{0};
  int pipelineInactiveCount_{0};
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
  using WriteCallback = std::function<Result(BytesPtr)>;

  MockTailHandler() = default;

  // TailEndpointHandler data methods
  Result onRead(TypeErasedBox&& msg) noexcept {
    messageCount_++;
    if (onReadCallback_) {
      return onReadCallback_(std::move(msg));
    }
    return messageResult_;
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    exceptionCount_++;
    if (onExceptionCallback_) {
      onExceptionCallback_(std::move(e));
    }
  }

  // Legacy EndpointHandler method (for backward compatibility)
  Result onMessage(TypeErasedBox&& msg) noexcept {
    return onRead(std::move(msg));
  }

  // EndpointHandlerLifecycle methods
  void handlerAdded() noexcept { handlerAddedCount_++; }
  void handlerRemoved() noexcept { handlerRemovedCount_++; }
  void onPipelineActive() noexcept { pipelineActiveCount_++; }
  void onPipelineInactive() noexcept { pipelineInactiveCount_++; }

  // Configuration
  void setMessageResult(Result result) { messageResult_ = result; }

  void setOnReadCallback(OnReadCallback cb) { onReadCallback_ = std::move(cb); }

  // Legacy aliases - TODO: Remove after tests are migrated
  void setOnMessageCallback(OnReadCallback cb) {
    setOnReadCallback(std::move(cb));
  }
  void setWriteCallback(WriteCallback cb) {
    onReadCallback_ = [cb = std::move(cb)](TypeErasedBox&& msg) {
      return cb(std::move(msg.get<BytesPtr>()));
    };
  }

  void setOnExceptionCallback(OnExceptionCallback cb) {
    onExceptionCallback_ = std::move(cb);
  }

  // Inspection
  int messageCount() const { return messageCount_; }
  int writeCount() const { return messageCount_; } // Legacy API TODO: Remove

  int exceptionCount() const { return exceptionCount_; }

  int handlerAddedCount() const { return handlerAddedCount_; }
  int handlerRemovedCount() const { return handlerRemovedCount_; }
  int pipelineActiveCount() const { return pipelineActiveCount_; }
  int pipelineInactiveCount() const { return pipelineInactiveCount_; }

  void reset() {
    messageCount_ = 0;
    exceptionCount_ = 0;
    messageResult_ = Result::Success;
    onReadCallback_ = nullptr;
    onExceptionCallback_ = nullptr;
    handlerAddedCount_ = 0;
    handlerRemovedCount_ = 0;
    pipelineActiveCount_ = 0;
    pipelineInactiveCount_ = 0;
  }

 private:
  int messageCount_{0};
  int exceptionCount_{0};
  Result messageResult_{Result::Success};
  OnReadCallback onReadCallback_;
  OnExceptionCallback onExceptionCallback_;
  int handlerAddedCount_{0};
  int handlerRemovedCount_{0};
  int pipelineActiveCount_{0};
  int pipelineInactiveCount_{0};
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
