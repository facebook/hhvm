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

#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>

THRIFT_FLAG_DEFINE_bool(rocket_server_disable_send_callback, false);

namespace apache::thrift::rocket {

class TimeoutCallback : public folly::HHWheelTimer::Callback {
 public:
  explicit TimeoutCallback(RocketStreamClientCallback& parent)
      : parent_(parent) {}
  void timeoutExpired() noexcept override { parent_.timeoutExpired(); }

 private:
  RocketStreamClientCallback& parent_;
};

RocketStreamClientCallback::RocketStreamClientCallback(
    StreamId streamId,
    IRocketServerConnection& connection,
    uint32_t initialRequestN,
    StreamMetricCallback& streamMetricCallback)
    : streamId_(streamId),
      connection_(connection),
      tokens_(initialRequestN),
      streamMetricCallback_(streamMetricCallback) {}

bool RocketStreamClientCallback::onFirstResponse(
    FirstResponsePayload&& firstResponse,
    folly::EventBase* /* unused */,
    StreamServerCallback* serverCallback) {
  if (UNLIKELY(serverCallbackOrCancelled_ == kCancelledFlag)) {
    streamMetricCallback_.onStreamCancel(rpcMethodName_);
    serverCallback->onStreamCancel();
    firstResponse.payload.reset();
    connection_.freeStream(streamId_, true /* complete */);
    return false;
  }

  streamMetricCallback_.onFirstResponse(rpcMethodName_);

  serverCallbackOrCancelled_ = reinterpret_cast<intptr_t>(serverCallback);
  if (UNLIKELY(connection_.areStreamsPaused())) {
    pauseStream();
  }

  DCHECK_NE(tokens_, 0u);
  int tokens = 0;
  if (--tokens_) {
    tokens = std::exchange(tokens_, 0);
  } else {
    scheduleTimeout();
  }

  firstResponse.metadata.streamId() = static_cast<uint32_t>(streamId_);

  sendPayload(
      std::move(firstResponse),
      /* next */ true,
      /* complete */ false,
      /* sendCallback */ nullptr);

  if (tokens) {
    return request(tokens);
  }
  return true;
}

void RocketStreamClientCallback::onFirstResponseError(
    folly::exception_wrapper ew) {
  streamMetricCallback_.onFirstResponseError(rpcMethodName_);
  bool isEncodedError = false;
  ew.handle(
      [this, &isEncodedError](RocketException& rex) {
        sendError(std::move(rex));
        isEncodedError = true;
      },
      [this, &isEncodedError](thrift::detail::EncodedFirstResponseError& err) {
        DCHECK(err.encoded.payload);
        isEncodedError = true;
        sendErrorPayload(std::move(err.encoded));
      });
  DCHECK(isEncodedError);

  // In case of a first response error ThriftServerRequestStream is
  // responsible for marking the request as complete (required for task
  // timeout support).
  connection_.freeStream(streamId_, false /* complete */);
}

bool RocketStreamClientCallback::onStreamNext(StreamPayload&& payload) {
  DCHECK_NE(tokens_, 0u);
  if (!--tokens_) {
    scheduleTimeout();
  }

  streamMetricCallback_.onStreamNext(rpcMethodName_);

  applyCompressionConfigIfNeeded(payload);

  sendStreamPayload(std::move(payload));

  return true;
}

void RocketStreamClientCallback::onStreamComplete() {
  streamMetricCallback_.onStreamComplete(rpcMethodName_);

  sendCompletePayload();

  connection_.freeStream(streamId_, /* complete */ true);
}

void RocketStreamClientCallback::onStreamError(folly::exception_wrapper ew) {
  streamMetricCallback_.onStreamError(rpcMethodName_);
  ew.handle(
      [this](RocketException& rex) {
        sendError(ErrorCode::APPLICATION_ERROR, rex.moveErrorData());
      },
      [this](::apache::thrift::detail::EncodedError& err) {
        sendError(ErrorCode::APPLICATION_ERROR, std::move(err.encoded));
      },
      [this](::apache::thrift::detail::EncodedStreamError& err) {
        applyCompressionConfigIfNeeded(err.encoded);
        sendErrorPayload(std::move(err.encoded));
      },
      [this, &ew](...) { sendError(ErrorCode::APPLICATION_ERROR, ew.what()); });
  connection_.freeStream(streamId_, true /* complete */);
}

bool RocketStreamClientCallback::onStreamHeaders(HeadersPayload&& payload) {
  ServerPushMetadata serverMeta;
  serverMeta.streamHeadersPush().ensure().streamId() =
      static_cast<uint32_t>(streamId_);
  serverMeta.streamHeadersPush()->headersPayloadContent() =
      std::move(payload.payload);
  connection_.sendMetadataPush(
      connection_.getPayloadSerializer()->packCompact(serverMeta));
  return true;
}

void RocketStreamClientCallback::resetServerCallback(
    StreamServerCallback& serverCallback) {
  serverCallbackOrCancelled_ = reinterpret_cast<intptr_t>(&serverCallback);
  if (UNLIKELY(connection_.areStreamsPaused())) {
    pauseStream();
  }
}

bool RocketStreamClientCallback::request(uint32_t tokens) {
  if (!tokens) {
    return true;
  }

  cancelTimeout();
  streamMetricCallback_.recordCreditsAvailable(rpcMethodName_, tokens_);
  streamMetricCallback_.onStreamRequestN(rpcMethodName_, tokens);
  tokens_ += tokens;
  return serverCallback()->onStreamRequestN(tokens);
}

void RocketStreamClientCallback::headers(HeadersPayload&& payload) {
  std::ignore = serverCallback()->onSinkHeaders(std::move(payload));
}

void RocketStreamClientCallback::pauseStream() {
  DCHECK(connection_.areStreamsPaused());
  if (UNLIKELY(!serverCallbackReady())) {
    return;
  }
  serverCallback()->pauseStream();
}

void RocketStreamClientCallback::resumeStream() {
  DCHECK(!connection_.areStreamsPaused());
  if (UNLIKELY(!serverCallbackReady())) {
    return;
  }
  serverCallback()->resumeStream();
}

void RocketStreamClientCallback::onStreamCancel() {
  streamMetricCallback_.onStreamCancel(rpcMethodName_);
  serverCallback()->onStreamCancel();
  if (contextStack_) {
    contextStack_->onStreamFinally(details::STREAM_ENDING_TYPES::CANCEL);
  }
}

void RocketStreamClientCallback::timeoutExpired() noexcept {
  DCHECK_EQ(0u, tokens_);

  serverCallback()->onStreamCancel();
  StreamRpcError streamRpcError;
  streamRpcError.code() = StreamRpcErrorCode::CREDIT_TIMEOUT;
  streamRpcError.name_utf8() =
      apache::thrift::TEnumTraits<StreamRpcErrorCode>::findName(
          StreamRpcErrorCode::CREDIT_TIMEOUT);
  streamRpcError.what_utf8() = "Stream expire timeout(no credit from client)";
  onStreamError(
      folly::make_exception_wrapper<rocket::RocketException>(
          rocket::ErrorCode::CANCELED,
          connection_.getPayloadSerializer()->packCompact(streamRpcError)));
}

void RocketStreamClientCallback::setProtoId(protocol::PROTOCOL_TYPES protoId) {
  protoId_ = protoId;
}

void RocketStreamClientCallback::setCompressionConfig(
    CompressionConfig compressionConfig) {
  compressionConfig_ = std::make_unique<CompressionConfig>(compressionConfig);
}

StreamServerCallback& RocketStreamClientCallback::getStreamServerCallback() {
  DCHECK(serverCallbackReady());
  return *serverCallback();
}

void RocketStreamClientCallback::scheduleTimeout() {
  if (!timeoutCallback_) {
    timeoutCallback_ = std::make_unique<TimeoutCallback>(*this);
  }
  connection_.scheduleStreamTimeout(timeoutCallback_.get());
  streamMetricCallback_.recordCreditsAvailable(rpcMethodName_, tokens_);
}

void RocketStreamClientCallback::cancelTimeout() {
  timeoutCallback_.reset();
}

void RocketStreamClientCallback::sendStreamPayload(StreamPayload&& payload) {
  sendPayload(
      std::move(payload),
      /* next */ true,
      /* complete */ false,
      makeSendCallback(/* endReason */ std::nullopt));
}

void RocketStreamClientCallback::sendCompletePayload() {
  connection_.sendPayload(
      streamId_,
      Payload::makeFromData(std::unique_ptr<folly::IOBuf>{}),
      Flags().complete(true),
      makeSendCallback(details::STREAM_ENDING_TYPES::COMPLETE));
}

void RocketStreamClientCallback::applyCompressionConfigIfNeeded(
    StreamPayload& payload) {
  if (compressionConfig_) {
    CompressionManager().setCompressionCodec(
        *compressionConfig_,
        payload.metadata,
        payload.payload ? payload.payload->computeChainDataLength() : 0);
  }
}
} // namespace apache::thrift::rocket
