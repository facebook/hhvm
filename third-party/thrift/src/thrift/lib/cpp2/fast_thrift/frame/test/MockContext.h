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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

#include <vector>

namespace apache::thrift::fast_thrift::frame::test {

/**
 * MockContext for testing pipeline handlers (InboundHandler-based).
 *
 * A standalone mock context for unit testing handlers in isolation
 * without building a full pipeline. Captures frames fired via fireRead()
 * and exceptions via fireException().
 *
 * Semantics:
 * - Result::Backpressure means "accepted but slow down" (frame is stored)
 * - Result::Error means "rejected" (frame is not stored)
 */
class MockContext {
 public:
  using Result = apache::thrift::fast_thrift::channel_pipeline::Result;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;

  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    // Always store the frame - Backpressure means "accepted but slow down"
    frames_.push_back(msg.take<BytesPtr>());
    if (returnBackpressure_) {
      return Result::Backpressure;
    }
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    // Store written frames - Backpressure means "accepted but slow down"
    writtenFrames_.push_back(msg.take<BytesPtr>());
    if (returnBackpressure_) {
      return Result::Backpressure;
    }
    return Result::Success;
  }

  BytesPtr allocate(size_t size) noexcept { return folly::IOBuf::create(size); }

  BytesPtr copyBuffer(const void* data, size_t size) noexcept {
    return folly::IOBuf::copyBuffer(data, size);
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  // Test configuration
  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  void setReturnError(bool value) { returnError_ = value; }

  // Test accessors
  const std::vector<BytesPtr>& frames() const { return frames_; }

  const std::vector<BytesPtr>& writtenFrames() const { return writtenFrames_; }

  const folly::exception_wrapper& exception() const { return exception_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  void reset() {
    frames_.clear();
    writtenFrames_.clear();
    exception_ = folly::exception_wrapper();
    returnBackpressure_ = false;
    returnError_ = false;
  }

 private:
  std::vector<BytesPtr> frames_;
  std::vector<BytesPtr> writtenFrames_;
  folly::exception_wrapper exception_;
  bool returnBackpressure_{false};
  bool returnError_{false};
};

} // namespace apache::thrift::fast_thrift::frame::test
