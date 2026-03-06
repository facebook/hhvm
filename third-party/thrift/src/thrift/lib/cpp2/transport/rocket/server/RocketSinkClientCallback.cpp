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

#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>

#include <memory>
#include <utility>

#include <fmt/core.h>
#include <glog/logging.h>

#include <folly/ExceptionWrapper.h>
#include <folly/Range.h>
#include <folly/ScopeGuard.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {
RocketSinkClientCallback::RocketSinkClientCallback(
    StreamId streamId,
    IRocketServerConnection& connection,
    uint32_t /*ignored*/)
    : streamId_(streamId), connection_(connection) {}

bool RocketSinkClientCallback::onFirstResponse(
    FirstResponsePayload&& firstResponse,
    folly::EventBase* /* unused */,
    SinkServerCallback* serverCallback) {
  if (UNLIKELY(serverCallbackOrError_ == kErrorFlag)) {
    serverCallback->onSinkError(TApplicationException(
        TApplicationException::TApplicationExceptionType::INTERRUPTION));
    firstResponse.payload.reset();
    connection_.freeStream(streamId_, true);
    return false;
  }

  serverCallbackOrError_ = reinterpret_cast<intptr_t>(serverCallback);

  connection_.sendPayload(
      streamId_,
      connection_.getPayloadSerializer()->pack(
          std::move(firstResponse),
          connection_.isDecodingMetadataUsingBinaryProtocol(),
          connection_.getRawSocket()),
      Flags().next(true));
  return true;
}

void RocketSinkClientCallback::onFirstResponseError(
    folly::exception_wrapper ew) {
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

  // In case of a first response error ThriftServerRequestSink is responsible
  // for marking the request as complete (required for task timeout support).
  connection_.freeStream(streamId_, false);
}

void RocketSinkClientCallback::onFinalResponse(StreamPayload&& finalResponse) {
  DCHECK(state_ == State::BothOpen || state_ == State::StreamOpen);

  // apply compression if client has specified compression codec
  if (compressionConfig_) {
    // There is a case when first response fails and final response callback is
    // triggered with nullptr payload which crashes the server. See S523649
    CompressionManager().setCompressionCodec(
        *compressionConfig_,
        finalResponse.metadata,
        finalResponse.payload ? finalResponse.payload->computeChainDataLength()
                              : 0);
  }

  connection_.sendPayload(
      streamId_,
      connection_.getPayloadSerializer()->pack(
          std::move(finalResponse),
          connection_.isDecodingMetadataUsingBinaryProtocol(),
          connection_.getRawSocket()),
      Flags().next(true).complete(true));
  auto state = state_;
  auto& connection = connection_;
  connection_.freeStream(streamId_, true);
  if (state == State::StreamOpen) {
    connection.decInflightFinalResponse();
  }
}

void RocketSinkClientCallback::onFinalResponseError(
    folly::exception_wrapper ew) {
  DCHECK(state_ == State::BothOpen || state_ == State::StreamOpen);
  ew.handle(
      [this](RocketException& rex) {
        connection_.sendError(
            streamId_,
            RocketException(ErrorCode::APPLICATION_ERROR, rex.moveErrorData()));
      },
      [this](::apache::thrift::detail::EncodedStreamError& err) {
        // apply compression if client has specified compression codec
        if (compressionConfig_) {
          CompressionManager().setCompressionCodec(
              *compressionConfig_,
              err.encoded.metadata,
              err.encoded.payload->computeChainDataLength());
        }
        connection_.sendPayload(
            streamId_,
            connection_.getPayloadSerializer()->pack(
                std::move(err.encoded),
                connection_.isDecodingMetadataUsingBinaryProtocol(),
                connection_.getRawSocket()),
            Flags().next(true).complete(true));
      },
      [&](...) {
        connection_.sendError(
            streamId_,
            RocketException(ErrorCode::APPLICATION_ERROR, ew.what()));
      });
  auto state = state_;
  auto& connection = connection_;

  // may destruct this instance
  connection_.freeStream(streamId_, true);

  if (state == State::StreamOpen) {
    // may destruct this instance
    connection.decInflightFinalResponse();
  }
}

bool RocketSinkClientCallback::onSinkRequestN(int32_t n) {
  if (timeout_) {
    timeout_->incCredits(n);
  }
  DCHECK(state_ == State::BothOpen);
  connection_.sendRequestN(streamId_, n);
  return true;
}

bool RocketSinkClientCallback::onSinkNext(StreamPayload&& payload) {
  if (state_ != State::BothOpen) {
    cancelTimeout();
    return false;
  }

  if (timeout_) {
    timeout_->decCredits();
  }

  return serverCallback()->onSinkNext(std::move(payload));
}

bool RocketSinkClientCallback::onSinkError(folly::exception_wrapper ew) {
  cancelTimeout();
  if (state_ != State::BothOpen) {
    return false;
  }
  serverCallback()->onSinkError(std::move(ew));
  return true;
}

bool RocketSinkClientCallback::onSinkComplete() {
  cancelTimeout();
  if (state_ != State::BothOpen) {
    return false;
  }
  state_ = State::StreamOpen;
  connection_.incInflightFinalResponse();
  return serverCallback()->onSinkComplete();
}

void RocketSinkClientCallback::setChunkTimeout(
    std::chrono::milliseconds timeout) {
  if (timeout != std::chrono::milliseconds::zero()) {
    timeout_ = std::make_unique<TimeoutCallback>(*this, timeout);
  }
}

void RocketSinkClientCallback::timeoutExpired() noexcept {
  auto ex = TApplicationException(
      TApplicationException::TApplicationExceptionType::TIMEOUT,
      "Sink chunk timeout");
  onSinkError(folly::make_exception_wrapper<TApplicationException>(ex));
  StreamRpcError streamRpcError;
  streamRpcError.code() = StreamRpcErrorCode::CHUNK_TIMEOUT;
  streamRpcError.name_utf8() =
      apache::thrift::TEnumTraits<StreamRpcErrorCode>::findName(
          StreamRpcErrorCode::CHUNK_TIMEOUT);
  streamRpcError.what_utf8() = "Sink chunk timeout";
  onFinalResponseError(
      folly::make_exception_wrapper<rocket::RocketException>(
          rocket::ErrorCode::CANCELED,
          connection_.getPayloadSerializer()->packCompact(streamRpcError)));
}

void RocketSinkClientCallback::setProtoId(protocol::PROTOCOL_TYPES protoId) {
  protoId_ = protoId;
}

void RocketSinkClientCallback::setCompressionConfig(
    CompressionConfig compressionConfig) {
  compressionConfig_ = std::make_unique<CompressionConfig>(compressionConfig);
}

void RocketSinkClientCallback::scheduleTimeout(
    std::chrono::milliseconds chunkTimeout) {
  if (timeout_) {
    connection_.scheduleSinkTimeout(timeout_.get(), chunkTimeout);
  }
}

void RocketSinkClientCallback::cancelTimeout() {
  if (timeout_) {
    timeout_->cancelTimeout();
  }
}

void RocketSinkClientCallback::TimeoutCallback::incCredits(uint64_t n) {
  if (credits_ == 0) {
    parent_.scheduleTimeout(chunkTimeout_);
  }
  credits_ += n;
}

void RocketSinkClientCallback::TimeoutCallback::decCredits() {
  DCHECK(credits_ != 0);
  if (--credits_ != 0) {
    parent_.scheduleTimeout(chunkTimeout_);
  } else {
    parent_.cancelTimeout();
  }
}

folly::Optional<Payload> RocketSinkClientCallback::bufferOrGetFullPayload(
    PayloadFrame&& payloadFrame) {
  if (payloadFrame.hasFollows()) {
    if (bufferedFragment_) {
      bufferedFragment_->append(std::move(payloadFrame.payload()));
    } else {
      bufferedFragment_ = std::move(payloadFrame.payload());
    }
    return folly::none;
  }

  if (bufferedFragment_) {
    auto full = std::move(*bufferedFragment_);
    bufferedFragment_.reset();
    full.append(std::move(payloadFrame.payload()));
    return full;
  }
  return std::move(payloadFrame.payload());
}

void RocketSinkClientCallback::handleFrame(PayloadFrame&& payloadFrame) {
  if (!serverCallbackReady()) {
    connection_.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId_),
                static_cast<uint8_t>(FrameType::PAYLOAD))));
    return;
  }

  const bool next = payloadFrame.hasNext();
  const bool complete = payloadFrame.hasComplete();
  auto fullPayload = bufferOrGetFullPayload(std::move(payloadFrame));
  if (!fullPayload) {
    return;
  }

  bool notViolateContract = true;
  if (next) {
    auto streamPayload =
        connection_.getPayloadSerializer()->unpack<StreamPayload>(
            std::move(*fullPayload),
            connection_.isDecodingMetadataUsingBinaryProtocol());
    if (streamPayload.hasException()) {
      notViolateContract = onSinkError(std::move(streamPayload.exception()));
      if (notViolateContract) {
        // freeStream may destroy this callback, return immediately.
        connection_.freeStream(streamId_, true);
        return;
      }
    } else {
      auto payloadMetadataRef = streamPayload->metadata.payloadMetadata();
      if (payloadMetadataRef &&
          payloadMetadataRef->getType() ==
              PayloadMetadata::Type::exceptionMetadata) {
        notViolateContract = onSinkError(
            apache::thrift::detail::EncodedStreamError(
                std::move(streamPayload.value())));
        if (notViolateContract) {
          // freeStream may destroy this callback, return immediately.
          connection_.freeStream(streamId_, true);
          return;
        }
      } else {
        notViolateContract = onSinkNext(std::move(*streamPayload));
      }
    }
  }

  if (complete && notViolateContract) {
    notViolateContract = onSinkComplete();
  }

  if (!notViolateContract) {
    connection_.close(
        folly::make_exception_wrapper<transport::TTransportException>(
            transport::TTransportException::TTransportExceptionType::
                STREAMING_CONTRACT_VIOLATION,
            "receiving sink payload frame after sink completion"));
  }
}

void RocketSinkClientCallback::handleFrame(ErrorFrame&& errorFrame) {
  if (!serverCallbackReady()) {
    if (errorFrame.errorCode() == ErrorCode::CANCELED) {
      earlyCancelled();
      return;
    }
    connection_.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId_),
                static_cast<uint8_t>(FrameType::ERROR))));
    return;
  }

  auto ew = [&] {
    if (errorFrame.errorCode() == ErrorCode::CANCELED) {
      return folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::TApplicationExceptionType::INTERRUPTION);
    } else {
      return folly::make_exception_wrapper<RocketException>(
          errorFrame.errorCode(), std::move(errorFrame.payload()).data());
    }
  }();

  bool notViolateContract = onSinkError(std::move(ew));
  if (notViolateContract) {
    connection_.freeStream(streamId_, true);
  } else {
    connection_.close(
        folly::make_exception_wrapper<transport::TTransportException>(
            transport::TTransportException::TTransportExceptionType::
                STREAMING_CONTRACT_VIOLATION,
            "receiving sink error frame after sink completion"));
  }
}

void RocketSinkClientCallback::handleFrame(RequestNFrame&&) {
  connection_.close(
      folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID,
          fmt::format(
              "Received unhandleable frame type ({}) for sink (id {})",
              static_cast<uint8_t>(FrameType::REQUEST_N),
              static_cast<uint32_t>(streamId_))));
}

void RocketSinkClientCallback::handleFrame(CancelFrame&&) {
  connection_.close(
      folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID,
          fmt::format(
              "Received unhandleable frame type ({}) for sink (id {})",
              static_cast<uint8_t>(FrameType::CANCEL),
              static_cast<uint32_t>(streamId_))));
}

void RocketSinkClientCallback::handleFrame(ExtFrame&&) {
  connection_.close(
      folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID,
          fmt::format(
              "Received unhandleable frame type ({}) for sink (id {})",
              static_cast<uint8_t>(FrameType::EXT),
              static_cast<uint32_t>(streamId_))));
}

void RocketSinkClientCallback::handleConnectionClose() {
  // TODO: Wire up call sites in RocketServerConnection to call this method
  // instead of inlining the onSinkError logic.
  bool state = onSinkError(TApplicationException(
      TApplicationException::TApplicationExceptionType::INTERRUPTION));
  DCHECK(state) << "onSinkError called after sink complete!";
}

} // namespace apache::thrift::rocket
