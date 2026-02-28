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

#include <cstdint>

namespace apache::thrift::rocket {

class BidirectionalStreamState {
 public:
  bool isTerminal() const {
    return state_ == State::CancelledEarly || state_ == State::Closed;
  }

  bool isAlive() const { return !isTerminal(); }

  bool isCancelledEarly() const { return state_ == State::CancelledEarly; }

  bool isAwaitingFirstResponse() const {
    return state_ == State::AwaitingFirstResponse;
  }

  bool isSinkOpen() const {
    return state_ == State::AwaitingFirstResponse ||
        state_ == State::StreamAndSinkOpen || state_ == State::OnlySinkOpen;
  }

  bool isStreamOpen() const {
    return state_ == State::AwaitingFirstResponse ||
        state_ == State::StreamAndSinkOpen || state_ == State::OnlyStreamOpen;
  }

  bool isAnyOpen() const {
    return state_ == State::AwaitingFirstResponse ||
        state_ == State::StreamAndSinkOpen || state_ == State::OnlyStreamOpen ||
        state_ == State::OnlySinkOpen;
  }

  bool isBothOpen() const {
    return state_ == State::AwaitingFirstResponse ||
        state_ == State::StreamAndSinkOpen;
  }

  bool firstResponseSent() const {
    return state_ != State::AwaitingFirstResponse &&
        state_ != State::CancelledEarly;
  }

  void onFirstResponseSent();
  void onFirstResponseError();

  void onStreamComplete();
  void onStreamError();
  void onStreamCancel();

  void onSinkComplete();
  void onSinkError();
  void onSinkCancel();

  void onCancelEarly();

 private:
  enum class State : uint8_t {
    AwaitingFirstResponse,
    CancelledEarly, // Terminal - cancelled before first response
    StreamAndSinkOpen,
    OnlyStreamOpen,
    OnlySinkOpen,
    Closed // Terminal
  };

  State state_{State::AwaitingFirstResponse};
};

} // namespace apache::thrift::rocket
