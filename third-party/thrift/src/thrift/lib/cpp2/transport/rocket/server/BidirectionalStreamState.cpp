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

#include <thrift/lib/cpp2/transport/rocket/server/BidirectionalStreamState.h>

#include <glog/logging.h>

namespace apache::thrift::rocket {

void BidirectionalStreamState::onFirstResponseSent() {
  DCHECK(state_ == State::AwaitingFirstResponse)
      << "First response can only be sent from AwaitingFirstResponse state";
  state_ = State::StreamAndSinkOpen;
}

void BidirectionalStreamState::onFirstResponseError() {
  DCHECK(state_ == State::AwaitingFirstResponse)
      << "First response error can only occur from AwaitingFirstResponse state";
  state_ = State::Closed;
}

void BidirectionalStreamState::onStreamComplete() {
  if (state_ == State::StreamAndSinkOpen) {
    state_ = State::OnlySinkOpen;
  } else if (state_ == State::OnlyStreamOpen) {
    state_ = State::Closed;
  } else {
    DCHECK(false) << "Stream must be open to be able to complete";
  }
}

void BidirectionalStreamState::onStreamError() {
  if (state_ == State::StreamAndSinkOpen) {
    state_ = State::OnlySinkOpen;
  } else if (state_ == State::OnlyStreamOpen) {
    state_ = State::Closed;
  } else {
    DCHECK(false) << "Stream must be open to be able to error";
  }
}

void BidirectionalStreamState::onStreamCancel() {
  if (state_ == State::StreamAndSinkOpen) {
    state_ = State::OnlySinkOpen;
  } else if (state_ == State::OnlyStreamOpen) {
    state_ = State::Closed;
  } else {
    DCHECK(false) << "Stream must be open to be able to be cancelled";
  }
}

void BidirectionalStreamState::onSinkComplete() {
  if (state_ == State::StreamAndSinkOpen) {
    state_ = State::OnlyStreamOpen;
  } else if (state_ == State::OnlySinkOpen) {
    state_ = State::Closed;
  } else {
    DCHECK(false) << "Sink must be open to be able to complete";
  }
}

void BidirectionalStreamState::onSinkError() {
  if (state_ == State::StreamAndSinkOpen) {
    state_ = State::OnlyStreamOpen;
  } else if (state_ == State::OnlySinkOpen) {
    state_ = State::Closed;
  } else {
    DCHECK(false) << "Sink must be open to be able to error";
  }
}

void BidirectionalStreamState::onSinkCancel() {
  if (state_ == State::StreamAndSinkOpen) {
    state_ = State::OnlyStreamOpen;
  } else if (state_ == State::OnlySinkOpen) {
    state_ = State::Closed;
  } else {
    DCHECK(false) << "Sink must be open to be able to be cancelled";
  }
}

void BidirectionalStreamState::onCancelEarly() {
  DCHECK(state_ == State::AwaitingFirstResponse)
      << "BiDi can only be cancelled early from AwaitingFirstResponse state";
  state_ = State::CancelledEarly;
}

} // namespace apache::thrift::rocket
