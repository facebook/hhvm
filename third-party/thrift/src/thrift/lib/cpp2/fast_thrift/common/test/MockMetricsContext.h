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

#include <vector>
#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace apache::thrift::fast_thrift::test {

// Lightweight mock context for testing metrics handlers in isolation.
// Tracks fireRead/fireWrite/fireException calls and lets tests control
// the returned Result.
class MockMetricsContext {
 public:
  channel_pipeline::Result fireRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return readResult_;
  }

  channel_pipeline::Result fireWrite(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return writeResult_;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void setReadResult(channel_pipeline::Result r) { readResult_ = r; }
  void setWriteResult(channel_pipeline::Result r) { writeResult_ = r; }

  auto& readMessages() { return readMessages_; }
  auto& writeMessages() { return writeMessages_; }
  bool hasException() const { return exception_.has_exception_ptr(); }
  const folly::exception_wrapper& exception() const { return exception_; }

 private:
  std::vector<channel_pipeline::TypeErasedBox> readMessages_;
  std::vector<channel_pipeline::TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  channel_pipeline::Result readResult_{channel_pipeline::Result::Success};
  channel_pipeline::Result writeResult_{channel_pipeline::Result::Success};
};

} // namespace apache::thrift::fast_thrift::test
