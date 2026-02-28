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

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>
#include <thrift/lib/cpp2/transport/rocket/server/BidirectionalStreamState.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>

namespace apache::thrift::rocket {

class RocketBiDiClientCallback final : public BiDiClientCallback {
 private:
  using EncodedFirstResponseError =
      ::apache::thrift::detail::EncodedFirstResponseError;
  using EncodedError = ::apache::thrift::detail::EncodedError;
  using EncodedStreamError = ::apache::thrift::detail::EncodedStreamError;

 public:
  RocketBiDiClientCallback(
      StreamId streamId,
      IRocketServerConnection& connection,
      int32_t initialTokens)
      : streamId_(streamId),
        connection_(connection),
        initialTokens_(initialTokens) {}

  void resetServerCallback(BiDiServerCallback& serverCallback) override {
    serverCallback_ = &serverCallback;
  }

  //
  //  BiDiClientCallback methods, called by server-side (bridge).
  //  These are "outgoing" methods, e.g. server calls client
  //

  bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* /* evb */,
      BiDiServerCallback* serverCallback) override;
  void onFirstResponseError(folly::exception_wrapper ew) override;
  bool onSinkCancel() override;
  bool onSinkRequestN(int32_t n) override;
  bool onStreamNext(StreamPayload&& payload) override;
  bool onStreamError(folly::exception_wrapper ew) override;
  bool onStreamComplete() override;

  //
  // end of BiDiClientCallback methods
  //

  //
  // Incoming methods: server receives client calls to forward them to server
  // callback
  //

  // Delegates to BiDiServerCallback::onStreamRequestN
  bool onStreamRequestN(int32_t n);

  // Delegates to BiDiServerCallback::onStreamCancel
  bool onStreamCancel();

  // Delegates to BiDiServerCallback::onSinkNext
  bool onSinkNext(StreamPayload&& payload);

  // Delegates to BiDiServerCallback::onSinkError
  bool onSinkError(folly::exception_wrapper ew);

  // Delegates to BiDiServerCallback::onSinkComplete
  bool onSinkComplete();

  //
  // end of Incoming methods
  //

  StreamId streamId() const { return streamId_; }

  void setCompressionConfig(CompressionConfig compressionConfig) {
    compressionConfig_ = compressionConfig;
  }

  bool serverCallbackReady() { return serverCallback_ != nullptr; }

  void cancelEarly() {
    DCHECK(!serverCallbackReady());
    state_.onCancelEarly();
  }

  bool isSinkOpen() const { return state_.isSinkOpen(); }

  bool isStreamOpen() const { return state_.isStreamOpen(); }

 private:
  const StreamId streamId_;
  IRocketServerConnection& connection_;
  BiDiServerCallback* serverCallback_{nullptr};

  BidirectionalStreamState state_;

  int32_t initialTokens_{0};

  std::optional<CompressionConfig> compressionConfig_;

  template <typename Payload>
  void sendPayload(Payload&& payload, bool next = false, bool complete = false);

  template <typename ErrorData>
  void sendError(ErrorCode errorCode, ErrorData errorData);

  void sendEmptyPayload(bool complete = true);

  void applyCompressionConfigIfNeeded(StreamPayload& payload);

  [[nodiscard]] bool freeStreamAndReturn(bool returnValue) {
    DCHECK(returnValue == false) << "Must return false when freeing the stream";
    connection_.freeStream(streamId_, /* markRequestComplete */ true);
    return returnValue;
  }
};

template <typename Payload>
void RocketBiDiClientCallback::sendPayload(
    Payload&& payload, bool next, bool complete) {
  auto serializer = connection_.getPayloadSerializer();
  auto rocketPayload = serializer->pack(
      std::move(payload),
      connection_.isDecodingMetadataUsingBinaryProtocol(),
      connection_.getRawSocket());
  connection_.sendPayload(
      streamId_,
      std::move(rocketPayload),
      Flags().next(next).complete(complete));
}

template <typename ErrorData>
void RocketBiDiClientCallback::sendError(
    ErrorCode errorCode, ErrorData errorData) {
  connection_.sendError(
      streamId_, RocketException(errorCode, std::move(errorData)));
}
} // namespace apache::thrift::rocket
