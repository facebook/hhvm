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
 * MockTailHandler satisfies EndpointHandler concept.
 * Generic tail endpoint mock for pipeline testing.
 * Records all message calls and allows configuring responses.
 */
class MockTailHandler {
 public:
  using WriteCallback = std::function<Result(BytesPtr)>;

  MockTailHandler() = default;

  Result onMessage(TypeErasedBox&& msg) noexcept {
    writeCount_++;
    auto bytes = std::move(msg.get<BytesPtr>());
    writtenBytes_.push_back(std::move(bytes));
    if (writeCallback_) {
      return writeCallback_(writtenBytes_.back()->clone());
    }
    return writeResult_;
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    exceptionCount_++;
    lastException_ = std::move(e);
  }

  // Configuration
  void setWriteResult(Result result) { writeResult_ = result; }

  void setWriteCallback(WriteCallback cb) { writeCallback_ = std::move(cb); }

  // Inspection
  int writeCount() const { return writeCount_; }

  int exceptionCount() const { return exceptionCount_; }

  const std::vector<BytesPtr>& writtenBytes() const { return writtenBytes_; }

  const folly::exception_wrapper& lastException() const {
    return lastException_;
  }

  void reset() {
    writeCount_ = 0;
    exceptionCount_ = 0;
    writtenBytes_.clear();
    writeResult_ = Result::Success;
    writeCallback_ = nullptr;
    lastException_ = {};
  }

 private:
  int writeCount_{0};
  int exceptionCount_{0};
  std::vector<BytesPtr> writtenBytes_;
  Result writeResult_{Result::Success};
  WriteCallback writeCallback_;
  folly::exception_wrapper lastException_;
};

/**
 * MockHeadHandler satisfies EndpointHandler concept.
 * Generic head endpoint mock for pipeline testing.
 * Records all messages received and allows configuring responses.
 */
class MockHeadHandler {
 public:
  using OnMessageCallback = std::function<Result(TypeErasedBox&&)>;
  using OnExceptionCallback = std::function<void(folly::exception_wrapper&&)>;

  MockHeadHandler() = default;

  Result onMessage(TypeErasedBox&& msg) noexcept {
    messageCount_++;
    if (onMessageCallback_) {
      return onMessageCallback_(std::move(msg));
    }
    return messageResult_;
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    exceptionCount_++;
    if (onExceptionCallback_) {
      onExceptionCallback_(std::move(e));
    }
  }

  // Configuration
  void setMessageResult(Result result) { messageResult_ = result; }

  void setOnMessageCallback(OnMessageCallback cb) {
    onMessageCallback_ = std::move(cb);
  }

  void setOnExceptionCallback(OnExceptionCallback cb) {
    onExceptionCallback_ = std::move(cb);
  }

  // Inspection
  int messageCount() const { return messageCount_; }

  int exceptionCount() const { return exceptionCount_; }

  void reset() {
    messageCount_ = 0;
    exceptionCount_ = 0;
    messageResult_ = Result::Success;
    onMessageCallback_ = nullptr;
    onExceptionCallback_ = nullptr;
  }

 private:
  int messageCount_{0};
  int exceptionCount_{0};
  Result messageResult_{Result::Success};
  OnMessageCallback onMessageCallback_;
  OnExceptionCallback onExceptionCallback_;
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
