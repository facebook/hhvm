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

#include <thrift/lib/cpp2/transport/core/SendCallbacks.h>

namespace apache::thrift {

void ResponseWriteTimeoutSendCallback::sendQueued() {
  connection_.scheduleTimeout(&timeout_, maxResponseWriteTime_);
  if (wrapped_ != nullptr) {
    wrapped_->sendQueued();
  }
}

void ResponseWriteTimeoutSendCallback::messageSent() {
  timeout_.cancelTimeout();
  if (wrapped_ != nullptr) {
    wrapped_->messageSent();
  }
  delete this;
}

void ResponseWriteTimeoutSendCallback::messageSendError(
    folly::exception_wrapper&& e) {
  timeout_.cancelTimeout();
  if (wrapped_ != nullptr) {
    wrapped_->messageSendError(std::move(e));
  }
  delete this;
}

void ResponseWriteTimeoutSendCallback::ResponseWriteTimeout::
    timeoutExpired() noexcept {
  connection_.dropConnection("Response write timeout expired");
}

} // namespace apache::thrift
