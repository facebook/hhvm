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

namespace {
class BiDiTimeoutCallback : public folly::HHWheelTimer::Callback {
 public:
  explicit BiDiTimeoutCallback(RocketBiDiClientCallback& parent)
      : parent_(parent) {}
  void timeoutExpired() noexcept override { parent_.timeoutExpired(); }

 private:
  RocketBiDiClientCallback& parent_;
};
} // namespace

//
//  BiDiClientCallback methods, called by server-side (bridge).
//  These are "outgoing" methods, e.g. server calls client
//

bool RocketBiDiClientCallback::onFirstResponse(
    FirstResponsePayload&& firstResponse,
    folly::EventBase* /* evb */,
    BiDiServerCallback* serverCallback) {
  if (state_.isCancelledEarly()) {
    if (serverCallback->onStreamCancel()) {
      std::ignore = serverCallback->onSinkComplete();
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

  if (UNLIKELY(connection_.areStreamsPaused())) {
    handlePausedByConnection();
  }

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
  cancelSinkTimeout();
  state_.onSinkCancel();
  connection_.sendCancel(streamId_);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onSinkRequestN(int32_t n) {
  if (sinkTimeout_) {
    sinkTimeout_->incCredits(n);
  }
  connection_.sendRequestN(streamId_, n);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamNext(StreamPayload&& payload) {
  DCHECK_NE(streamTokens_, 0u);
  if (!--streamTokens_) {
    scheduleTimeout();
  }

  applyCompressionConfigIfNeeded(payload);
  sendPayload(std::move(payload), /* next */ true);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamError(folly::exception_wrapper ew) {
  cancelTimeout();
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
  cancelTimeout();
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
  if (n <= 0) {
    return !serverCallback_->onStreamRequestN(n);
  }
  cancelTimeout();
  streamTokens_ += n;
  return serverCallback_->onStreamRequestN(n);
}

bool RocketBiDiClientCallback::onStreamCancel() {
  DCHECK(state_.isStreamOpen());
  cancelTimeout();

  state_.onStreamCancel();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onStreamCancel()) {
    if (wasTerminal) {
      serverCallback_ = nullptr;
      return freeStreamAndReturn(false);
    }
    return false;
  }
  if (wasTerminal) {
    serverCallback_ = nullptr;
    return freeStreamAndReturn(false);
  }
  DCHECK(state_.isAlive());
  return true;
}

bool RocketBiDiClientCallback::onSinkNext(StreamPayload&& payload) {
  DCHECK(state_.isSinkOpen());

  if (sinkTimeout_) {
    sinkTimeout_->decCredits();
  }

  return serverCallback_->onSinkNext(std::move(payload));
}

bool RocketBiDiClientCallback::onSinkError(folly::exception_wrapper ew) {
  DCHECK(state_.isSinkOpen());
  cancelSinkTimeout();

  state_.onSinkError();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onSinkError(std::move(ew))) {
    if (wasTerminal) {
      serverCallback_ = nullptr;
      return freeStreamAndReturn(false);
    }
    return false;
  }
  if (wasTerminal) {
    serverCallback_ = nullptr;
    return freeStreamAndReturn(false);
  }
  DCHECK(state_.isAlive());
  return true;
}

bool RocketBiDiClientCallback::onSinkComplete() {
  DCHECK(state_.isSinkOpen());
  cancelSinkTimeout();

  state_.onSinkComplete();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onSinkComplete()) {
    if (wasTerminal) {
      serverCallback_ = nullptr;
      return freeStreamAndReturn(false);
    }
    return false;
  }
  if (wasTerminal) {
    serverCallback_ = nullptr;
    return freeStreamAndReturn(false);
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
      // As noted in `RocketClient::handleSinkResponse`, bidi streams
      // currently lack the codegen to be able to support FD passing.
      DCHECK(
          !streamPayload->metadata.fdMetadata().has_value() ||
          streamPayload->metadata.fdMetadata()->numFds().value_or(0) == 0)
          << "FD passing is not implemented for bidi streams";

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

void RocketBiDiClientCallback::handlePausedByConnection() {
  DCHECK(connection_.areStreamsPaused());
  if (UNLIKELY(!serverCallbackReady())) {
    return;
  }
  serverCallback_->pauseStream();
}

void RocketBiDiClientCallback::handleResumedByConnection() {
  DCHECK(!connection_.areStreamsPaused());
  if (UNLIKELY(!serverCallbackReady())) {
    return;
  }
  serverCallback_->resumeStream();
}

void RocketBiDiClientCallback::handleConnectionClose() {
  cancelTimeout();
  cancelSinkTimeout();
  // Null out serverCallback_ before calling through it. This prevents
  // use-after-free if the bridge's onSinkError destroys the server callback
  // (the subsequent onStreamCancel would otherwise call a dangling pointer).
  // It also prevents reentrant handleFrame calls from seeing a stale pointer.
  auto* cb = std::exchange(serverCallback_, nullptr);
  if (!cb) {
    return;
  }

  // During connection close, notify the server callback directly rather than
  // going through onSinkError/onStreamCancel to avoid state machine DCHECKs
  // when both transitions happen in sequence (terminal state after second).
  // The connection handles cleanup (freeStream, requestComplete) separately.
  if (isSinkOpen()) {
    std::ignore = cb->onSinkError(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::INTERRUPTION));
  }
  if (isStreamOpen()) {
    std::ignore = cb->onStreamCancel();
  }
}

void RocketBiDiClientCallback::timeoutExpired() noexcept {
  DCHECK_EQ(0u, streamTokens_);
  cancelSinkTimeout();

  // Credit timeout is fatal for the entire bidi interaction.
  // Null out serverCallback_ before calling through it. This prevents
  // use-after-free if the bridge's onSinkError destroys the server callback
  // (the subsequent onStreamCancel would otherwise call a dangling pointer).
  auto* cb = std::exchange(serverCallback_, nullptr);
  if (!cb) {
    return;
  }

  // Notify sink error through the Stapler BEFORE cancelling the stream.
  // Cancelling the stream triggers cleanup of the sink bridge's input
  // generator, which posts a deferred canceled() callback. If the sink is
  // already cancelled through the Stapler, the bridge's clientCb_ will be
  // null and the deferred callback is a no-op. Without this ordering,
  // freeStream() destroys this callback while the deferred callback still
  // holds a reference, causing a use-after-free.
  bool sinkWasOpen = isSinkOpen();
  if (sinkWasOpen) {
    state_.onSinkCancel();
    std::ignore = cb->onSinkError(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::TIMEOUT,
            "BiDi stream credit timeout"));
  }
  if (isStreamOpen()) {
    state_.onStreamCancel();
    std::ignore = cb->onStreamCancel();
  }

  // Send cancel for the sink direction so the client stops sending.
  if (sinkWasOpen) {
    connection_.sendCancel(streamId_);
  }

  // Send CREDIT_TIMEOUT error to the client and free the stream.
  StreamRpcError streamRpcError;
  streamRpcError.code() = StreamRpcErrorCode::CREDIT_TIMEOUT;
  streamRpcError.name_utf8() =
      apache::thrift::TEnumTraits<StreamRpcErrorCode>::findName(
          StreamRpcErrorCode::CREDIT_TIMEOUT);
  streamRpcError.what_utf8() = "BiDi stream credit timeout";
  connection_.sendError(
      streamId_,
      RocketException(
          ErrorCode::CANCELED,
          connection_.getPayloadSerializer()->packCompact(streamRpcError)));
  connection_.freeStream(streamId_, /* markRequestComplete */ true);
}

void RocketBiDiClientCallback::scheduleTimeout() {
  if (!timeoutCallback_) {
    timeoutCallback_ = std::make_unique<BiDiTimeoutCallback>(*this);
  }
  connection_.scheduleStreamTimeout(timeoutCallback_.get());
}

void RocketBiDiClientCallback::cancelTimeout() {
  timeoutCallback_.reset();
}

void RocketBiDiClientCallback::setChunkTimeout(
    std::chrono::milliseconds timeout) {
  if (timeout != std::chrono::milliseconds::zero()) {
    sinkTimeout_ = std::make_unique<SinkTimeoutCallback>(*this, timeout);
  }
}

void RocketBiDiClientCallback::sinkChunkTimeoutExpired() noexcept {
  cancelSinkTimeout();

  // Chunk timeout is fatal for the entire bidi interaction.
  // Null out serverCallback_ before calling through it. This prevents
  // use-after-free if the bridge's onSinkError destroys the server callback
  // (the subsequent onStreamCancel would otherwise call a dangling pointer).
  auto* cb = std::exchange(serverCallback_, nullptr);
  if (!cb) {
    return;
  }

  // Cancel the sink half first. We must not delegate to onSinkError() because
  // it calls freeStreamAndReturn() when both halves are closed, which would
  // destroy `this` before we finish cleanup.
  bool sinkWasOpen = state_.isSinkOpen();
  if (sinkWasOpen) {
    state_.onSinkCancel();
    std::ignore = cb->onSinkError(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::TIMEOUT,
            "Sink chunk timeout"));
  }

  // Cancel the stream half if still open.
  if (state_.isStreamOpen()) {
    cancelTimeout();
    state_.onStreamCancel();
    std::ignore = cb->onStreamCancel();
  }

  // Send cancel for the sink direction so the client stops sending.
  if (sinkWasOpen) {
    connection_.sendCancel(streamId_);
  }

  // Send CHUNK_TIMEOUT error to the client and free the stream.
  StreamRpcError streamRpcError;
  streamRpcError.code() = StreamRpcErrorCode::CHUNK_TIMEOUT;
  streamRpcError.name_utf8() =
      apache::thrift::TEnumTraits<StreamRpcErrorCode>::findName(
          StreamRpcErrorCode::CHUNK_TIMEOUT);
  streamRpcError.what_utf8() = "Sink chunk timeout";
  connection_.sendError(
      streamId_,
      RocketException(
          ErrorCode::CANCELED,
          connection_.getPayloadSerializer()->packCompact(streamRpcError)));
  connection_.freeStream(streamId_, /* markRequestComplete */ true);
}

void RocketBiDiClientCallback::scheduleSinkTimeout(
    std::chrono::milliseconds chunkTimeout) {
  if (sinkTimeout_) {
    connection_.scheduleSinkTimeout(sinkTimeout_.get(), chunkTimeout);
  }
}

void RocketBiDiClientCallback::cancelSinkTimeout() {
  if (sinkTimeout_) {
    sinkTimeout_->cancelTimeout();
  }
}

void RocketBiDiClientCallback::SinkTimeoutCallback::incCredits(uint64_t n) {
  if (credits_ == 0) {
    parent_.scheduleSinkTimeout(chunkTimeout_);
  }
  credits_ += n;
}

void RocketBiDiClientCallback::SinkTimeoutCallback::decCredits() {
  DCHECK(credits_ != 0);
  if (--credits_ != 0) {
    parent_.scheduleSinkTimeout(chunkTimeout_);
  } else {
    parent_.cancelSinkTimeout();
  }
}

} // namespace apache::thrift::rocket
