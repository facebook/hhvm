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
#include <folly/io/async/HHWheelTimer.h>

#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>

namespace folly {
class EventBase;
} // namespace folly

namespace apache {
namespace thrift {
namespace rocket {

class RocketStreamClientCallback final : public StreamClientCallback {
 public:
  RocketStreamClientCallback(
      StreamId streamId,
      RocketServerConnection& connection,
      uint32_t initialRequestN);
  ~RocketStreamClientCallback() override = default;

  bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* evb,
      StreamServerCallback* serverCallback) override;
  void onFirstResponseError(folly::exception_wrapper ew) override;

  bool onStreamNext(StreamPayload&& payload) override;
  void onStreamError(folly::exception_wrapper ew) override;
  void onStreamComplete() override;
  bool onStreamHeaders(HeadersPayload&& payload) override;

  void resetServerCallback(StreamServerCallback&) override;

  bool request(uint32_t n);
  void onStreamCancel();
  void headers(HeadersPayload&& payload);
  void pauseStream();
  void resumeStream();

  StreamServerCallback& getStreamServerCallback();
  void timeoutExpired() noexcept;
  void setProtoId(protocol::PROTOCOL_TYPES);
  void setCompressionConfig(CompressionConfig compressionConfig);
  bool serverCallbackReady() const {
    return serverCallbackOrCancelled_ != kCancelledFlag && serverCallback();
  }
  void earlyCancelled() {
    DCHECK(!serverCallbackReady());
    serverCallbackOrCancelled_ = kCancelledFlag;
  }

 private:
  StreamServerCallback* serverCallback() const {
    return reinterpret_cast<StreamServerCallback*>(serverCallbackOrCancelled_);
  }

  const StreamId streamId_;
  RocketServerConnection& connection_;
  static constexpr intptr_t kCancelledFlag = 1;
  intptr_t serverCallbackOrCancelled_{0};
  uint64_t tokens_{0};
  std::unique_ptr<folly::HHWheelTimer::Callback> timeoutCallback_;
  protocol::PROTOCOL_TYPES protoId_;
  std::unique_ptr<CompressionConfig> compressionConfig_;

  void scheduleTimeout();
  void cancelTimeout();
};

} // namespace rocket
} // namespace thrift
} // namespace apache
