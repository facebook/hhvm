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

#include <functional>
#include <memory>
#include <utility>

#include <glog/logging.h>

#include <folly/ExceptionWrapper.h>
#include <folly/Range.h>
#include <folly/ScopeGuard.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/PayloadUtils.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>

namespace apache {
namespace thrift {
namespace rocket {

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
    uint32_t initialRequestN)
    : streamId_(streamId), connection_(connection), tokens_(initialRequestN) {}

bool RocketStreamClientCallback::onFirstResponse(
    FirstResponsePayload&& firstResponse,
    folly::EventBase* /* unused */,
    StreamServerCallback* serverCallback) {
  if (UNLIKELY(serverCallbackOrCancelled_ == kCancelledFlag)) {
    serverCallback->onStreamCancel();
    firstResponse.payload.reset();
    connection_.freeStream(streamId_, true);
    return false;
  }

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

  firstResponse.metadata.streamId_ref() = static_cast<uint32_t>(streamId_);

  connection_.sendPayload(
      streamId_, pack(std::move(firstResponse)), Flags().next(true));

  if (tokens) {
    return request(tokens);
  }
  return true;
}

void RocketStreamClientCallback::onFirstResponseError(
    folly::exception_wrapper ew) {
  bool isEncodedError = ew.with_exception<RocketException>([&](auto& ex) {
    connection_.sendError(streamId_, std::move(ex));
  }) ||
      ew.with_exception<thrift::detail::EncodedFirstResponseError>(
          [&](auto& encodedError) {
            DCHECK(encodedError.encoded.payload);
            connection_.sendPayload(
                streamId_,
                pack(std::move(encodedError.encoded)),
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
    apache::thrift::rocket::detail::setCompressionCodec(
        *compressionConfig_,
        payload.metadata,
        payload.payload ? payload.payload->computeChainDataLength() : 0);
  }

  connection_.sendPayload(
      streamId_, pack(std::move(payload)), Flags().next(true));

  return true;
}

void RocketStreamClientCallback::onStreamComplete() {
  connection_.sendPayload(
      streamId_,
      Payload::makeFromData(std::unique_ptr<folly::IOBuf>{}),
      Flags().complete(true));
  connection_.freeStream(streamId_, true);
}

void RocketStreamClientCallback::onStreamError(folly::exception_wrapper ew) {
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
          apache::thrift::rocket::detail::setCompressionCodec(
              *compressionConfig_,
              err.encoded.metadata,
              err.encoded.payload->computeChainDataLength());
        }
        connection_.sendPayload(
            streamId_,
            pack(std::move(err.encoded)),
            Flags().next(true).complete(true));
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
  serverMeta.streamHeadersPush_ref().ensure().streamId_ref() =
      static_cast<uint32_t>(streamId_);
  serverMeta.streamHeadersPush_ref()->headersPayloadContent_ref() =
      std::move(payload.payload);
  connection_.sendMetadataPush(packCompact(std::move(serverMeta)));
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
  serverCallback()->onStreamCancel();
}

void RocketStreamClientCallback::timeoutExpired() noexcept {
  DCHECK_EQ(0u, tokens_);

  serverCallback()->onStreamCancel();
  StreamRpcError streamRpcError;
  streamRpcError.code_ref() = StreamRpcErrorCode::CREDIT_TIMEOUT;
  streamRpcError.name_utf8_ref() =
      apache::thrift::TEnumTraits<StreamRpcErrorCode>::findName(
          StreamRpcErrorCode::CREDIT_TIMEOUT);
  streamRpcError.what_utf8_ref() =
      "Stream expire timeout(no credit from client)";
  onStreamError(folly::make_exception_wrapper<rocket::RocketException>(
      rocket::ErrorCode::CANCELED, packCompact(streamRpcError)));
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
}

void RocketStreamClientCallback::cancelTimeout() {
  timeoutCallback_.reset();
}
} // namespace rocket
} // namespace thrift
} // namespace apache
