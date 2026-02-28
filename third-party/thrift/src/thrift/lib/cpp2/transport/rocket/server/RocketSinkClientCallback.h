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
#include <memory>

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/HHWheelTimer.h>

#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>

namespace apache::thrift::rocket {

class RocketSinkClientCallback final : public SinkClientCallback {
 public:
  explicit RocketSinkClientCallback(
      StreamId streamId,
      IRocketServerConnection& connection,
      uint32_t /*ignored*/ = 0);
  ~RocketSinkClientCallback() override = default;
  bool onFirstResponse(
      FirstResponsePayload&&, folly::EventBase*, SinkServerCallback*) override;
  void onFirstResponseError(folly::exception_wrapper) override;

  void onFinalResponse(StreamPayload&&) override;
  void onFinalResponseError(folly::exception_wrapper) override;

  bool onSinkRequestN(int32_t) override;

  bool onSinkNext(StreamPayload&&);
  bool onSinkError(folly::exception_wrapper);
  bool onSinkComplete();

  void setChunkTimeout(std::chrono::milliseconds timeout);
  void timeoutExpired() noexcept;
  void setProtoId(protocol::PROTOCOL_TYPES);
  void setCompressionConfig(CompressionConfig compressionConfig);
  bool serverCallbackReady() const {
    return serverCallbackOrError_ != kErrorFlag && serverCallback();
  }
  void earlyCancelled() {
    DCHECK(!serverCallbackReady());
    serverCallbackOrError_ = kErrorFlag;
  }

  void resetServerCallback(SinkServerCallback& serverCallback) override {
    DCHECK(serverCallbackReady());
    serverCallbackOrError_ = reinterpret_cast<intptr_t>(&serverCallback);
  }

 private:
  SinkServerCallback* serverCallback() const {
    return reinterpret_cast<SinkServerCallback*>(serverCallbackOrError_);
  }

  class TimeoutCallback : public folly::HHWheelTimer::Callback {
   public:
    explicit TimeoutCallback(
        RocketSinkClientCallback& parent,
        std::chrono::milliseconds chunkTimeout)
        : parent_(parent), chunkTimeout_(chunkTimeout) {
      DCHECK(chunkTimeout != std::chrono::milliseconds::zero());
    }
    void timeoutExpired() noexcept override { parent_.timeoutExpired(); }
    void incCredits(uint64_t n);
    void decCredits();

   private:
    RocketSinkClientCallback& parent_;
    std::chrono::milliseconds chunkTimeout_;
    uint64_t credits_{0};
  };

  void scheduleTimeout(std::chrono::milliseconds chunkTimeout);
  void cancelTimeout();

  enum class State { BothOpen, StreamOpen };
  State state_{State::BothOpen};
  const StreamId streamId_;
  IRocketServerConnection& connection_;
  static constexpr intptr_t kErrorFlag = 1;
  intptr_t serverCallbackOrError_{0};
  std::unique_ptr<TimeoutCallback> timeout_;
  protocol::PROTOCOL_TYPES protoId_;
  std::unique_ptr<CompressionConfig> compressionConfig_;
};

} // namespace apache::thrift::rocket
