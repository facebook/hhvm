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

#include <thrift/lib/cpp2/transport/rocket/client/RocketStreamServerCallbackWithChunkTimeout.h>

#include <folly/io/async/HHWheelTimer.h>

#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>

namespace apache::thrift::rocket {

namespace {
template <typename ServerCallback>
class TimeoutCallback : public folly::HHWheelTimer::Callback {
 public:
  explicit TimeoutCallback(ServerCallback& parent) : parent_(parent) {}
  void timeoutExpired() noexcept override { parent_.timeoutExpired(); }

 private:
  ServerCallback& parent_;
};
} // namespace

bool RocketStreamServerCallbackWithChunkTimeout::onStreamRequestN(
    uint64_t tokens) {
  if (credits_ == 0) {
    scheduleTimeout();
  }
  credits_ += tokens;
  return RocketStreamServerCallback::onStreamRequestN(tokens);
}

bool RocketStreamServerCallbackWithChunkTimeout::onInitialPayload(
    FirstResponsePayload&& payload, folly::EventBase* evb) {
  if (credits_ > 0) {
    scheduleTimeout();
  }
  return RocketStreamServerCallback::onInitialPayload(std::move(payload), evb);
}

StreamChannelStatusResponse
RocketStreamServerCallbackWithChunkTimeout::onStreamPayload(
    StreamPayload&& payload) {
  DCHECK(credits_ != 0);
  if (--credits_ != 0) {
    scheduleTimeout();
  } else {
    cancelTimeout();
  }
  return RocketStreamServerCallback::onStreamPayload(std::move(payload));
}

void RocketStreamServerCallbackWithChunkTimeout::timeoutExpired() noexcept {
  onStreamError(folly::make_exception_wrapper<transport::TTransportException>(
      transport::TTransportException::TTransportExceptionType::TIMED_OUT,
      folly::to<std::string>(
          "stream chunk timeout after ", chunkTimeout_.count(), " ms.")));
  onStreamCancel();
}

void RocketStreamServerCallbackWithChunkTimeout::scheduleTimeout() {
  if (!timeout_) {
    timeout_ = std::make_unique<
        TimeoutCallback<RocketStreamServerCallbackWithChunkTimeout>>(*this);
  }
  client_.scheduleTimeout(timeout_.get(), chunkTimeout_);
}

void RocketStreamServerCallbackWithChunkTimeout::cancelTimeout() {
  timeout_.reset();
}
} // namespace apache::thrift::rocket
