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
#include <thrift/lib/cpp/StreamEventHandler.h>

namespace apache::thrift {
namespace details {

enum class BIDI_FINISH_REASON {
  COMPLETE = 0, // Both input and output completed normally
  CANCEL = 1, // Client cancelled
  ERROR = 2, // Error occurred
};

} // namespace details

class BiDiEventHandler {
 public:
  BiDiEventHandler() = default;
  BiDiEventHandler(const BiDiEventHandler&) = delete;
  BiDiEventHandler& operator=(const BiDiEventHandler&) = delete;
  BiDiEventHandler(BiDiEventHandler&&) = delete;
  BiDiEventHandler& operator=(BiDiEventHandler&&) = delete;
  virtual ~BiDiEventHandler() {}

  // Called once when the BiDi RPC is established (before any sink/stream)
  virtual void onBiDiSubscribe(void*) {}

  // Sink (from client to server) events
  virtual void onBiDiSinkNext(void*) {}
  virtual void onBiDiSinkCredit(void*, uint32_t) {}

  // Stream (from server to client) events
  virtual void onBiDiStreamNext(void*) {}
  virtual void onBiDiStreamCredit(void*, uint32_t) {}
  virtual void onBiDiStreamPause(void*, details::STREAM_PAUSE_REASON) {}

  // Error handling for both directions
  virtual void handleBiDiSinkError(void*, const folly::exception_wrapper&) {}
  virtual void handleBiDiStreamError(void*, const folly::exception_wrapper&) {}

  // Called once when the entire BiDi RPC completes (both sink and stream)
  virtual void onBiDiFinally(void*, details::BIDI_FINISH_REASON) {}
};

} // namespace apache::thrift
