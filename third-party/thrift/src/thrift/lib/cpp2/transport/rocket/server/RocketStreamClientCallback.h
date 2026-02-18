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

#include <folly/io/async/HHWheelTimer.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>

THRIFT_FLAG_DECLARE(rocket_server_disable_send_callback, bool);

namespace apache::thrift::rocket {

class RocketServerConnection;

class RocketStreamClientCallback final : public StreamClientCallback {
 public:
  RocketStreamClientCallback(
      StreamId streamId,
      IRocketServerConnection& connection,
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

  bool handle(RequestNFrame requestNFrame);
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

  void setRpcMethodName(std::string rpcMethodName) {
    rpcMethodName_ = std::move(rpcMethodName);
  }
  std::string_view getRpcMethodName() const { return rpcMethodName_; }

  void setContextStack(std::shared_ptr<ContextStack> contextStack) {
    contextStack_ = std::move(contextStack);
  }

  StreamId streamId() const { return streamId_; }

 private:
  StreamServerCallback* serverCallback() const {
    return reinterpret_cast<StreamServerCallback*>(serverCallbackOrCancelled_);
  }

  class StreamMessageSentCallback
      : public apache::thrift::MessageChannel::SendCallback {
   public:
    explicit StreamMessageSentCallback(
        std::shared_ptr<ContextStack> contextStack,
        std::shared_ptr<uint64_t> chunksInMemory)
        : contextStack_(std::move(contextStack)),
          endReason_(std::nullopt),
          chunksInMemory_(std::move(chunksInMemory)) {}

    explicit StreamMessageSentCallback(
        std::shared_ptr<ContextStack> contextStack,
        std::optional<apache::thrift::details::STREAM_ENDING_TYPES>&& endReason,
        std::shared_ptr<uint64_t> chunksInMemory)
        : contextStack_(std::move(contextStack)),
          endReason_(std::move(endReason)),
          chunksInMemory_(std::move(chunksInMemory)) {}

    void sendQueued() noexcept override {}

    void messageSent() noexcept override {
      invokeCallbacks();
      delete this;
    }

    void messageSendError(folly::exception_wrapper&&) noexcept override {
      invokeCallbacks();
      delete this;
    }

   private:
    void invokeCallbacks() {
      if (chunksInMemory_ && *chunksInMemory_ > 0) {
        --(*chunksInMemory_);
      }
      if (contextStack_) {
        if (endReason_) {
          contextStack_->onStreamFinally(*endReason_);
        } else {
          contextStack_->onStreamNextSent();
        }
      }
    }
    std::shared_ptr<ContextStack> contextStack_;
    std::optional<details::STREAM_ENDING_TYPES> endReason_;
    std::shared_ptr<uint64_t> chunksInMemory_;
  };

  using SendCallbackPtr = apache::thrift::MessageChannel::SendCallbackPtr;
  SendCallbackPtr makeSendCallback(
      std::optional<details::STREAM_ENDING_TYPES> endReason) {
    // Always create callback to track chunksInMemory_ for debugging credit
    // timeouts. Pass contextStack_ only if enabled.
    auto contextStackForCallback =
        (contextStack_ && !THRIFT_FLAG(rocket_server_disable_send_callback))
        ? contextStack_
        : nullptr;
    return SendCallbackPtr(new StreamMessageSentCallback(
        std::move(contextStackForCallback),
        std::move(endReason),
        chunksInMemory_));
  }

  template <typename Payload>
  void sendPayload(
      Payload&& payload,
      bool next,
      bool complete,
      apache::thrift::MessageChannel::SendCallbackPtr sendCallback);

  void sendStreamPayload(StreamPayload&& payload);

  template <typename Payload>
  void sendErrorPayload(Payload&& payload);

  void sendCompletePayload();

  template <typename ErrorData>
  void sendError(ErrorCode errorCode, ErrorData errorData);

  inline void sendError(RocketException&& rex);

  void applyCompressionConfigIfNeeded(StreamPayload& payload);

  const StreamId streamId_;
  IRocketServerConnection& connection_;
  static constexpr intptr_t kCancelledFlag = 1;
  intptr_t serverCallbackOrCancelled_{0};
  uint64_t tokens_{0};
  std::unique_ptr<folly::HHWheelTimer::Callback> timeoutCallback_;
  protocol::PROTOCOL_TYPES protoId_;
  std::unique_ptr<CompressionConfig> compressionConfig_;
  std::string rpcMethodName_{"<unknown_stream_method>"};
  std::shared_ptr<ContextStack> contextStack_{nullptr};

  // Tracks chunks in memory (received but not yet sent).
  // Uses shared_ptr so callback can safely decrement after stream destruction.
  std::shared_ptr<uint64_t> chunksInMemory_ = std::make_shared<uint64_t>(0);

  void scheduleTimeout();
  void cancelTimeout();
};

template <typename Payload>
void RocketStreamClientCallback::sendPayload(
    Payload&& payload,
    bool next,
    bool complete,
    apache::thrift::MessageChannel::SendCallbackPtr sendCallback) {
  auto serializer = connection_.getPayloadSerializer();
  auto rocketPayload = serializer->pack(
      std::forward<Payload>(payload),
      connection_.isDecodingMetadataUsingBinaryProtocol(),
      connection_.getRawSocket());

  connection_.sendPayload(
      streamId_,
      std::move(rocketPayload),
      Flags().next(next).complete(complete),
      std::move(sendCallback));
}

template <typename ErrorData>
void RocketStreamClientCallback::sendError(
    ErrorCode errorCode, ErrorData errorData) {
  sendError(RocketException(errorCode, std::move(errorData)));
}

void RocketStreamClientCallback::sendError(RocketException&& rex) {
  connection_.sendError(
      streamId_,
      std::move(rex),
      makeSendCallback(details::STREAM_ENDING_TYPES::ERROR));
}

template <typename Payload>
void RocketStreamClientCallback::sendErrorPayload(Payload&& payload) {
  sendPayload(
      std::forward<Payload>(payload),
      true /* next */,
      true /* complete */,
      makeSendCallback(details::STREAM_ENDING_TYPES::ERROR));
}

} // namespace apache::thrift::rocket
