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

#include <memory>
#include <utility>

#include <glog/logging.h>

#include <folly/ExceptionWrapper.h>
#include <folly/Range.h>
#include <folly/ScopeGuard.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>

THRIFT_FLAG_DEFINE_bool(rocket_server_disable_send_callback, false);

namespace apache::thrift::rocket {
namespace {
class StreamNextSentCallback
    : public apache::thrift::MessageChannel::SendCallback {
 public:
  explicit StreamNextSentCallback(std::shared_ptr<ContextStack> contextStack)
      : contextStack_(std::move(contextStack)) {}

  void sendQueued() noexcept override {}

  void messageSent() noexcept override {
    if (contextStack_) {
      contextStack_->onStreamNextSent();
    }
    delete this;
  }

  void messageSendError(folly::exception_wrapper&&) noexcept override {
    if (contextStack_) {
      contextStack_->onStreamNextSent();
    }
    delete this;
  }

 private:
  std::shared_ptr<ContextStack> contextStack_;
};
} // namespace

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
    RocketServerConnection& connection,
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
    connection_.freeStream(streamId_, true);
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

  connection_.sendPayload(
      streamId_,
      connection_.getPayloadSerializer()->pack(
          std::move(firstResponse),
          connection_.isDecodingMetadataUsingBinaryProtocol(),
          connection_.getRawSocket()),
      Flags().next(true));

  if (tokens) {
    return request(tokens);
  }
  return true;
}

void RocketStreamClientCallback::onFirstResponseError(
    folly::exception_wrapper ew) {
  streamMetricCallback_.onFirstResponseError(rpcMethodName_);
  bool isEncodedError = ew.with_exception<RocketException>([&](auto& ex) {
    connection_.sendError(streamId_, std::move(ex));
  }) ||
      ew.with_exception<thrift::detail::EncodedFirstResponseError>(
          [&](auto& encodedError) {
            DCHECK(encodedError.encoded.payload);
            connection_.sendPayload(
                streamId_,
                connection_.getPayloadSerializer()->pack(
                    std::move(encodedError.encoded),
                    connection_.isDecodingMetadataUsingBinaryProtocol(),
                    connection_.getRawSocket()),
                Flags().next(true).complete(true));
          });
  DCHECK(isEncodedError);

  // In case of a first response error ThriftServerRequestStream is responsible
  // for marking the request as complete (required for task timeout support).
  connection_.freeStream(streamId_, false);
}

bool RocketStreamClientCallback::onStreamNext(StreamPayload&& payload) {
  DCHECK_NE(tokens_, 0u);
  if (!--tokens_) {
    scheduleTimeout();
  }

  // apply compression if client has specified compression codec
  if (compressionConfig_) {
    CompressionManager().setCompressionCodec(
        *compressionConfig_,
        payload.metadata,
        payload.payload ? payload.payload->computeChainDataLength() : 0);
  }

  streamMetricCallback_.onStreamNext(rpcMethodName_);

  apache::thrift::MessageChannel::SendCallbackPtr sendCallback = nullptr;
  if (contextStack_ && !THRIFT_FLAG(rocket_server_disable_send_callback)) {
    sendCallback = apache::thrift::MessageChannel::SendCallbackPtr(
        new StreamNextSentCallback(contextStack_));
  }

  connection_.sendPayload(
      streamId_,
      connection_.getPayloadSerializer()->pack(
          std::move(payload),
          connection_.isDecodingMetadataUsingBinaryProtocol(),
          connection_.getRawSocket()),
      Flags().next(true),
      std::move(sendCallback));

  return true;
}

void RocketStreamClientCallback::onStreamComplete() {
  streamMetricCallback_.onStreamComplete(rpcMethodName_);

  connection_.sendPayload(
      streamId_,
      Payload::makeFromData(std::unique_ptr<folly::IOBuf>{}),
      Flags().complete(true));
  connection_.freeStream(streamId_, true);
}

void RocketStreamClientCallback::onStreamError(folly::exception_wrapper ew) {
  streamMetricCallback_.onStreamError(rpcMethodName_);
  ew.handle(
      [this](RocketException& rex) {
        connection_.sendError(
            streamId_,
            RocketException(ErrorCode::APPLICATION_ERROR, rex.moveErrorData()));
      },
      [this](::apache::thrift::detail::EncodedError& err) {
        connection_.sendError(
            streamId_,
            RocketException(
                ErrorCode::APPLICATION_ERROR, std::move(err.encoded)));
      },
      [this](::apache::thrift::detail::EncodedStreamError& err) {
        // apply compression if client has specified compression codec
        if (compressionConfig_) {
          rocket::CompressionManager().setCompressionCodec(
              *compressionConfig_,
              err.encoded.metadata,
              err.encoded.payload->computeChainDataLength());
        }

        apache::thrift::MessageChannel::SendCallbackPtr sendCallback = nullptr;
        if (contextStack_ &&
            !THRIFT_FLAG(rocket_server_disable_send_callback)) {
          sendCallback = apache::thrift::MessageChannel::SendCallbackPtr(
              new StreamNextSentCallback(contextStack_));
        }

        connection_.sendPayload(
            streamId_,
            connection_.getPayloadSerializer()->pack(
                std::move(err.encoded),
                connection_.isDecodingMetadataUsingBinaryProtocol(),
                connection_.getRawSocket()),
            Flags().next(true).complete(true),
            std::move(sendCallback));
      },
      [this, &ew](...) {
        connection_.sendError(
            streamId_,
            RocketException(ErrorCode::APPLICATION_ERROR, ew.what()));
      });
  connection_.freeStream(streamId_, true);
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
  CHECK(serverCallbackReady()) << serverCallbackOrCancelled_;
  streamMetricCallback_.onStreamCancel(rpcMethodName_);
  serverCallback()->onStreamCancel();
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
  onStreamError(folly::make_exception_wrapper<rocket::RocketException>(
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
} // namespace apache::thrift::rocket
