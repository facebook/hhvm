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

#include <thrift/lib/cpp2/transport/rocket/server/RocketBiDiClientCallback.h>

#include <fmt/core.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

//
//  BiDiClientCallback methods, called by server-side (bridge).
//  These are "outgoing" methods, e.g. server calls client
//

bool RocketBiDiClientCallback::onFirstResponse(
    FirstResponsePayload&& firstResponse,
    folly::EventBase* /* evb */,
    BiDiServerCallback* serverCallback) {
  if (state_.isCancelledEarly()) {
    if (serverCallback_->onStreamCancel()) {
      std::ignore = serverCallback_->onSinkComplete();
    }

    DCHECK(state_.isTerminal());
    return freeStreamAndReturn(false);
  }

  DCHECK(state_.isAwaitingFirstResponse());

  DCHECK(serverCallback_ == nullptr) << "Server callback must not be set yet";
  serverCallback_ = serverCallback;

  firstResponse.metadata.streamId() = static_cast<uint32_t>(streamId_);
  sendPayload(std::move(firstResponse), /* next= */ true);
  state_.onFirstResponseSent();

  if (initialTokens_ > 0) {
    onStreamRequestN(initialTokens_);
  }

  DCHECK(state_.isAlive());
  return true;
}

void RocketBiDiClientCallback::onFirstResponseError(
    folly::exception_wrapper ew) {
  state_.onFirstResponseError();
  ew.handle(
      [this](RocketException& rex) {
        connection_.sendError(streamId_, std::move(rex));
      },
      [this](EncodedFirstResponseError& err) {
        DCHECK(err.encoded.payload);
        sendPayload(err.encoded, /* next */ true, /* complete */ true);
      });

  DCHECK(state_.isTerminal());
  std::ignore = freeStreamAndReturn(state_.isAlive());
}

bool RocketBiDiClientCallback::onSinkCancel() {
  if (!state_.isSinkOpen()) {
    return state_.isAlive();
  }
  state_.onSinkCancel();
  connection_.sendCancel(streamId_);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onSinkRequestN(int32_t n) {
  connection_.sendRequestN(streamId_, n);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamNext(StreamPayload&& payload) {
  applyCompressionConfigIfNeeded(payload);
  sendPayload(std::move(payload), /* next */ true);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamError(folly::exception_wrapper ew) {
  state_.onStreamError();
  ew.handle(
      [this](RocketException& rex) {
        sendError(ErrorCode::APPLICATION_ERROR, rex.moveErrorData());
      },
      [this](EncodedError& err) {
        sendError(ErrorCode::APPLICATION_ERROR, std::move(err.encoded));
      },
      [this](EncodedStreamError& err) {
        applyCompressionConfigIfNeeded(err.encoded);
        sendPayload(
            std::move(err.encoded), /* next */ true, /* complete */ true);
      },
      [this, &ew](...) { sendError(ErrorCode::APPLICATION_ERROR, ew.what()); });

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamComplete() {
  state_.onStreamComplete();
  sendEmptyPayload(/* complete */ true);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

//
// end of BiDiClientCallback methods
//

//
// Incoming methods: server receives client calls to forward them to server
// callback
//

bool RocketBiDiClientCallback::onStreamRequestN(int32_t n) {
  DCHECK(state_.isStreamOpen());
  return !serverCallback_->onStreamRequestN(n);
}

bool RocketBiDiClientCallback::onStreamCancel() {
  DCHECK(state_.isStreamOpen());

  state_.onStreamCancel();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onStreamCancel()) {
    if (wasTerminal) {
      return freeStreamAndReturn(false);
    }
    return false;
  }
  DCHECK(state_.isAlive());
  return true;
}

bool RocketBiDiClientCallback::onSinkNext(StreamPayload&& payload) {
  DCHECK(state_.isSinkOpen());

  return serverCallback_->onSinkNext(std::move(payload));
}

bool RocketBiDiClientCallback::onSinkError(folly::exception_wrapper ew) {
  DCHECK(state_.isSinkOpen());

  state_.onSinkError();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onSinkError(std::move(ew))) {
    if (wasTerminal) {
      return freeStreamAndReturn(false);
    }
    return false;
  }
  DCHECK(state_.isAlive());
  return true;
}

bool RocketBiDiClientCallback::onSinkComplete() {
  DCHECK(state_.isSinkOpen());

  state_.onSinkComplete();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onSinkComplete()) {
    if (wasTerminal) {
      return freeStreamAndReturn(false);
    }
    return false;
  }
  DCHECK(state_.isAlive());
  return true;
}

//
// end of Incoming methods
//

void RocketBiDiClientCallback::sendEmptyPayload(bool complete) {
  connection_.sendPayload(
      streamId_,
      Payload::makeFromData(std::unique_ptr<folly::IOBuf>{}),
      Flags().complete(complete));
}

void RocketBiDiClientCallback::applyCompressionConfigIfNeeded(
    StreamPayload& payload) {
  if (compressionConfig_) {
    CompressionManager().setCompressionCodec(
        *compressionConfig_,
        payload.metadata,
        payload.payload ? payload.payload->computeChainDataLength() : 0);
  }
}

folly::Optional<Payload> RocketBiDiClientCallback::bufferOrGetFullPayload(
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

void RocketBiDiClientCallback::handleFrame(PayloadFrame&& payloadFrame) {
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

  bool alive = true;
  if (next) {
    auto streamPayload =
        connection_.getPayloadSerializer()->unpack<StreamPayload>(
            std::move(*fullPayload),
            connection_.isDecodingMetadataUsingBinaryProtocol());

    if (streamPayload.hasException()) {
      if (isSinkOpen()) {
        alive = onSinkError(std::move(streamPayload.exception()));
      }
    } else {
      auto payloadMetadataRef = streamPayload->metadata.payloadMetadata();
      if (payloadMetadataRef &&
          payloadMetadataRef->getType() ==
              PayloadMetadata::Type::exceptionMetadata) {
        if (isSinkOpen()) {
          alive = onSinkError(
              apache::thrift::detail::EncodedStreamError(
                  std::move(streamPayload.value())));
        }
      } else {
        if (isSinkOpen()) {
          alive = onSinkNext(std::move(*streamPayload));
        }
      }
    }
  }

  if (complete && alive && isSinkOpen()) {
    std::ignore = onSinkComplete();
  }
}

void RocketBiDiClientCallback::handleFrame(ErrorFrame&& errorFrame) {
  if (!serverCallbackReady()) {
    if (errorFrame.errorCode() == ErrorCode::CANCELED) {
      cancelEarly();
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

  if (isSinkOpen()) {
    std::ignore = onSinkError(std::move(ew));
  }
}

void RocketBiDiClientCallback::handleFrame(RequestNFrame&& requestNFrame) {
  if (!serverCallbackReady()) {
    connection_.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId_),
                static_cast<uint8_t>(FrameType::REQUEST_N))));
    return;
  }

  if (isStreamOpen()) {
    onStreamRequestN(requestNFrame.requestN());
  }
}

void RocketBiDiClientCallback::handleFrame(CancelFrame&&) {
  if (!serverCallbackReady()) {
    connection_.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId_),
                static_cast<uint8_t>(FrameType::CANCEL))));
    return;
  }

  if (isStreamOpen()) {
    onStreamCancel();
  }
}

void RocketBiDiClientCallback::handleFrame(ExtFrame&& extFrame) {
  if (!serverCallbackReady()) {
    connection_.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId_),
                static_cast<uint8_t>(FrameType::EXT))));
    return;
  }

  if (!extFrame.hasIgnore()) {
    connection_.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unsupported EXT frame type ({}) for stream (id {})",
                static_cast<uint32_t>(extFrame.extFrameType()),
                static_cast<uint32_t>(streamId_))));
  }
}

void RocketBiDiClientCallback::handleConnectionClose() {
  if (!serverCallbackReady()) {
    return;
  }

  // During connection close, notify the server callback directly rather than
  // going through onSinkError/onStreamCancel to avoid state machine DCHECKs
  // when both transitions happen in sequence (terminal state after second).
  // The connection handles cleanup (freeStream, requestComplete) separately.
  if (isSinkOpen()) {
    std::ignore = serverCallback_->onSinkError(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::INTERRUPTION));
  }
  if (isStreamOpen()) {
    std::ignore = serverCallback_->onStreamCancel();
  }
}

} // namespace apache::thrift::rocket
