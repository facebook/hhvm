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
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>

#include <mutex>
#include <vector>

namespace apache::thrift::fast_thrift::test {

/**
 * TestServerAppAdapter implements ServerInboundAppAdapter for server-side
 * testing.
 *
 * This adapter receives messages from the pipeline and provides:
 * - Message tracking for test verification
 * - Synchronization primitives for async test coordination
 * - Configurable echo-back behavior
 * - Configurable result return for backpressure testing
 */
class TestServerAppAdapter {
 public:
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
  using Result = apache::thrift::fast_thrift::channel_pipeline::Result;

  TestServerAppAdapter() = default;

  // --- TailEndpointHandler lifecycle ---

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

  // --- TailEndpointHandler Interface ---

  /**
   * Called by the pipeline when a message reaches the application layer.
   * This is invoked after the message passes through all pipeline handlers.
   */
  void onException(folly::exception_wrapper&& ew) noexcept {
    std::lock_guard lock(messagesMutex_);
    lastException_ = std::move(ew);
  }

  Result onRead(TypeErasedBox&& msg) noexcept {
    std::lock_guard lock(messagesMutex_);
    messageCount_++;
    messages_.push_back(std::move(msg));
    if (!batonPosted_) {
      batonPosted_ = true;
      messageBaton_.post();
    }

    if (echoEnabled_ && !messages_.back().empty()) {
      auto& bytes = messages_.back().get<BytesPtr>();
      auto response = bytes->clone();
      auto responseBox = TypeErasedBox(std::move(response));
      (void)pipeline_->fireWrite(std::move(responseBox));
    }

    return onReadResult_;
  }

  // --- Configuration ---

  /**
   * Set the pipeline reference for echo-back functionality.
   */
  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline) {
    pipeline_ = pipeline;
  }

  /**
   * Set the result to return from onRead.
   */
  void setOnReadResult(Result result) { onReadResult_ = result; }

  /**
   * Enable or disable echo-back behavior.
   */
  void setEchoEnabled(bool enabled) { echoEnabled_ = enabled; }

  // --- Test Inspection ---

  /**
   * Get the number of messages received via onRead.
   */
  int messageCount() const {
    std::lock_guard lock(messagesMutex_);
    return messageCount_;
  }

  /**
   * Get the list of received messages.
   */
  const std::vector<TypeErasedBox>& messages() const { return messages_; }

  /**
   * Wait for at least one message to be received.
   * Returns false if timeout expires.
   */
  bool waitForMessage(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
    return messageBaton_.try_wait_for(timeout);
  }

  /**
   * Reset the message baton for waiting on the next message.
   */
  void resetMessageBaton() {
    std::lock_guard lock(messagesMutex_);
    batonPosted_ = false;
    messageBaton_.reset();
  }

  /**
   * Reset all state for reuse in tests.
   */
  void reset() {
    std::lock_guard lock(messagesMutex_);
    messageCount_ = 0;
    messages_.clear();
    batonPosted_ = false;
    messageBaton_.reset();
    onReadResult_ = Result::Success;
    echoEnabled_ = false;
  }

 private:
  // TailEndpointHandler state
  mutable std::mutex messagesMutex_;
  int messageCount_{0};
  std::vector<TypeErasedBox> messages_;
  bool batonPosted_{false};
  folly::Baton<> messageBaton_;
  Result onReadResult_{Result::Success};

  // Exception state
  folly::exception_wrapper lastException_;

  // Echo configuration
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  bool echoEnabled_{false};
};

} // namespace apache::thrift::fast_thrift::test
