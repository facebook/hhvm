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

namespace apache::thrift::fast_thrift::common::test {

/**
 * MockTransportAdapter satisfies OutboundTransportHandler concept.
 * Used for testing transport-side pipeline integration.
 */
class MockTransportAdapter {
 public:
  using WriteCallback =
      std::function<channel_pipeline::Result(channel_pipeline::BytesPtr)>;

  MockTransportAdapter() = default;

  channel_pipeline::Result onMessage(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    writeCount_++;
    auto bytes = std::move(msg.get<channel_pipeline::BytesPtr>());
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

  void pauseRead() noexcept {
    pauseReadCount_++;
    paused_ = true;
  }

  void resumeRead() noexcept {
    resumeReadCount_++;
    paused_ = false;
  }

  void setWriteResult(channel_pipeline::Result result) {
    writeResult_ = result;
  }

  void setWriteCallback(WriteCallback cb) { writeCallback_ = std::move(cb); }

  int writeCount() const { return writeCount_; }

  int exceptionCount() const { return exceptionCount_; }

  int pauseReadCount() const { return pauseReadCount_; }

  int resumeReadCount() const { return resumeReadCount_; }

  bool isPaused() const { return paused_; }

  const std::vector<channel_pipeline::BytesPtr>& writtenBytes() const {
    return writtenBytes_;
  }

  const folly::exception_wrapper& lastException() const {
    return lastException_;
  }

  void reset() {
    writeCount_ = 0;
    exceptionCount_ = 0;
    pauseReadCount_ = 0;
    resumeReadCount_ = 0;
    paused_ = false;
    writtenBytes_.clear();
    writeResult_ = channel_pipeline::Result::Success;
    writeCallback_ = nullptr;
    lastException_ = {};
  }

 private:
  int writeCount_{0};
  int exceptionCount_{0};
  int pauseReadCount_{0};
  int resumeReadCount_{0};
  bool paused_{false};
  std::vector<channel_pipeline::BytesPtr> writtenBytes_;
  channel_pipeline::Result writeResult_{channel_pipeline::Result::Success};
  WriteCallback writeCallback_;
  folly::exception_wrapper lastException_;
};

} // namespace apache::thrift::fast_thrift::common::test
