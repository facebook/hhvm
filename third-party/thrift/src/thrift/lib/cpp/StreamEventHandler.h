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

#include <chrono>
#include <optional>
#include <folly/ExceptionWrapper.h>

namespace apache::thrift {
namespace details {
enum class STREAM_ENDING_TYPES {
  COMPLETE = 0,
  ERROR = 1,
  CANCEL = 2,
};

enum class SINK_ENDING_TYPES {
  COMPLETE = 0,
  COMPLETE_WITH_ERROR = 1,
  ERROR = 2,
};
} // namespace details

// EXPERIMENTAL: DO NOT USE WITHOUT TALKING TO THRIFT TEAM
class StreamEventHandler {
 public:
  struct StreamContext {
    std::optional<std::chrono::steady_clock::time_point>
        interactionCreationTime;
  };

  virtual ~StreamEventHandler() {}
  virtual void onStreamSubscribe(void*) {}
  virtual void onStreamNext(void*) {}
  virtual void onStreamCredit(void*, uint32_t) {}
  virtual void onStreamPauseReceive(void*) {}
  virtual void onStreamResumeReceive(void*) {}
  virtual void handleStreamErrorWrapped(
      void*, const folly::exception_wrapper&) {}
  virtual void onStreamFinally(void*, details::STREAM_ENDING_TYPES) {}

  virtual void onSinkSubscribe(void*, const StreamContext&) {}
  virtual void onSinkNext(void*) {}
  virtual void onSinkConsumed(void*) {}
  virtual void onSinkCancel(void*) {}
  virtual void onSinkCredit(void*, uint32_t) {}
  virtual void handleSinkError(void*, const folly::exception_wrapper&) {}
  virtual void onSinkFinally(void*, details::SINK_ENDING_TYPES) {}
};

} // namespace apache::thrift
