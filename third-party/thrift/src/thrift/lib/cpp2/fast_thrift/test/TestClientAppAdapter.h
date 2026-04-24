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
#include <folly/io/async/EventBase.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>

#include <mutex>
#include <vector>

namespace apache::thrift::fast_thrift::test {

/**
 * TestClientAppAdapter implements ClientInboundAppAdapter for client-side
 * testing.
 *
 * This adapter is designed for client-side testing:
 * - Sends messages to the server via the pipeline
 * - Receives responses from the server
 * - Provides synchronization for request-response patterns
 * - Configurable result return for backpressure testing
 */
class TestClientAppAdapter {
 public:
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
  using Result = apache::thrift::fast_thrift::channel_pipeline::Result;

  TestClientAppAdapter() = default;

  // --- TailEndpointHandler lifecycle ---

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  // --- TailEndpointHandler Interface ---

  /**
   * Called by the pipeline when a response message is received from the server.
   */
  void onException(folly::exception_wrapper&& ew) noexcept {
    std::lock_guard lock(responsesMutex_);
    lastException_ = std::move(ew);
  }

  Result onRead(TypeErasedBox&& msg) noexcept {
    std::lock_guard lock(responsesMutex_);
    responseCount_++;
    responses_.push_back(std::move(msg));
    if (!batonPosted_) {
      batonPosted_ = true;
      responseBaton_.post();
    }
    return onReadResult_;
  }

  // --- Client Send API ---

  /**
   * Send a message to the server via the pipeline.
   * Dispatches the write to the EventBase thread for thread safety.
   */
  Result send(std::unique_ptr<folly::IOBuf> data) {
    if (!pipeline_ || !evb_) {
      return Result::Error;
    }

    Result result = Result::Success;
    evb_->runInEventBaseThreadAndWait([this, &data, &result]() {
      std::lock_guard lock(responsesMutex_);
      sentCount_++;

      auto msg = TypeErasedBox(std::move(data));
      result = pipeline_->fireWrite(std::move(msg));
    });

    return result;
  }

  /**
   * Send a string message to the server.
   */
  Result sendString(std::string_view data) {
    return send(folly::IOBuf::copyBuffer(data.data(), data.size()));
  }

  // --- Configuration ---

  /**
   * Set the pipeline reference for sending messages.
   */
  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline) {
    pipeline_ = pipeline;
  }

  /**
   * Set the EventBase for dispatching writes.
   */
  void setEventBase(folly::EventBase* evb) { evb_ = evb; }

  /**
   * Set the result to return from onRead.
   */
  void setOnReadResult(Result result) { onReadResult_ = result; }

  // --- Test Inspection ---

  /**
   * Get the number of responses received.
   */
  int responseCount() const {
    std::lock_guard lock(responsesMutex_);
    return responseCount_;
  }

  /**
   * Get the number of messages sent.
   */
  int sentCount() const {
    std::lock_guard lock(responsesMutex_);
    return sentCount_;
  }

  /**
   * Get the list of received responses.
   */
  const std::vector<TypeErasedBox>& responses() const { return responses_; }

  /**
   * Wait for at least one response to be received.
   * Returns false if timeout expires.
   */
  bool waitForResponse(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
    return responseBaton_.try_wait_for(timeout);
  }

  /**
   * Reset the response baton for waiting on the next response.
   */
  void resetResponseBaton() {
    std::lock_guard lock(responsesMutex_);
    batonPosted_ = false;
    responseBaton_.reset();
  }

  /**
   * Reset all state for reuse in tests.
   */
  void reset() {
    std::lock_guard lock(responsesMutex_);
    responseCount_ = 0;
    sentCount_ = 0;
    responses_.clear();
    batonPosted_ = false;
    responseBaton_.reset();
    onReadResult_ = Result::Success;
  }

 private:
  // Client state
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  folly::EventBase* evb_{nullptr};
  mutable std::mutex responsesMutex_;
  int responseCount_{0};
  int sentCount_{0};
  std::vector<TypeErasedBox> responses_;
  folly::exception_wrapper lastException_;
  bool batonPosted_{false};
  folly::Baton<> responseBaton_;
  Result onReadResult_{Result::Success};
};

} // namespace apache::thrift::fast_thrift::test
