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

#include <thrift/lib/cpp2/transport/rocket/server/RefactoredRocketServerConnection.h>

#include <memory>
#include <utility>

// Ensure RocketServerHandler is fully defined before template instantiation
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>

#include <fmt/core.h>
#include <folly/ExceptionString.h>
#include <folly/ExceptionWrapper.h>
#include <folly/GLog.h>
#include <folly/Likely.h>
#include <folly/MapUtil.h>
#include <folly/Overload.h>
#include <folly/ScopeGuard.h>
#include <folly/SocketAddress.h>
#include <folly/Utility.h>
#include <folly/dynamic.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>

#include <wangle/acceptor/ConnectionManager.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketBiDiClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionPlugins.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/OutgoingFrameHandler.h>

THRIFT_FLAG_DECLARE_bool(enable_rocket_connection_observers);
THRIFT_FLAG_DECLARE_bool(thrift_enable_stream_counters);
THRIFT_FLAG_DEFINE_bool(rocket_use_outgoing_frame_handler, true);

namespace apache::thrift::rocket {

RefactoredRocketServerConnection::RefactoredRocketServerConnection(
    folly::AsyncTransport::UniquePtr socket,
    std::unique_ptr<RocketServerHandler> frameHandler,
    MemoryTracker& ingressMemoryTracker,
    MemoryTracker& egressMemoryTracker,
    StreamMetricCallback& streamMetricCallback,
    const IRocketServerConnection::Config& cfg)
    : IRocketServerConnection(),
      evb_(*socket->getEventBase()),
      socket_(std::move(socket)),
      rawSocket_(
          socket_ ? socket_->getUnderlyingTransport<folly::AsyncSocket>()
                  : nullptr),
      parser_(*this, cfg.parserStrategy, cfg.parserAllocator),
      frameHandler_(std::move(frameHandler)),
      streamStarvationTimeout_(cfg.streamStarvationTimeout),
      egressBufferBackpressureThreshold_(cfg.egressBufferBackpressureThreshold),
      egressBufferRecoverySize_(
          cfg.egressBufferBackpressureThreshold *
          cfg.egressBufferBackpressureRecoveryFactor),
      socketDrainer_(*this),
      ingressMemoryTracker_(ingressMemoryTracker),
      egressMemoryTracker_(egressMemoryTracker),
      streamMetricCallback_(streamMetricCallback),
      connectionAdapter_(*this),
      writeBatcher_(
          connectionAdapter_,
          cfg.writeBatchingInterval,
          cfg.writeBatchingSize,
          cfg.writeBatchingByteSize),
      writerCallback_(connectionAdapter_),
      bufferCallback_(connectionAdapter_),
      keepAliveHandler_(connectionAdapter_),
      setupFrameAcceptor_(connectionAdapter_, *frameHandler_),
      requestResponseHandler_(&connectionAdapter_),
      requestFnfHandler_(&connectionAdapter_),
      metadataPushHandler_(connectionAdapter_),
      streamCallbackManager_(&connectionAdapter_),
      requestStreamHandler_(&connectionAdapter_),
      existingStreamFrameHandler_(&connectionAdapter_),
      requestChannelHandler_(&connectionAdapter_),
      incomingFrameHandler_(
          connectionAdapter_,
          setupFrameAcceptor_,
          requestResponseHandler_,
          requestFnfHandler_,
          keepAliveHandler_,
          metadataPushHandler_,
          streamCallbackManager_,
          requestStreamHandler_,
          requestChannelHandler_,
          existingStreamFrameHandler_) {
  CHECK(socket_);
  CHECK(frameHandler_);

  peerAddress_ = socket_->getPeerAddress();

  socket_->setReadCB(&parser_);
  if (rawSocket_) {
    rawSocket_->setBufferCallback(&bufferCallback_);
    rawSocket_->setSendTimeout(cfg.socketWriteTimeout.count());

    if (cfg.socketOptions != nullptr) {
      auto sockfd = rawSocket_->getNetworkSocket();
      for (auto& [option, value] : *cfg.socketOptions) {
        if (option.apply(sockfd, value)) {
          folly::SocketAddress address;
          rawSocket_->getAddress(&address);
          FB_LOG_EVERY_MS(WARNING, 60 * 1000) << fmt::format(
              "Could not apply SocketOption(level={}, optname={}, value={}) to socket {}",
              option.level,
              option.optname,
              value.toString(),
              address.describe());
        }
      }
    }
  }

  // To prevent a C++ intialization error we need to initialize the the observer
  // container after the vtable is fully created
  this->initializeObserverContainer();
}

namespace {
StreamMetricCallback& getNoopStreamMetricCallback() {
  static folly::Indestructible<NoopStreamMetricCallback>
      kNoopStreamMetricCallback;
  return *kNoopStreamMetricCallback;
}
} // namespace

RocketStreamClientCallback* FOLLY_NULLABLE
RefactoredRocketServerConnection::createStreamClientCallback(
    StreamId streamId,
    IRocketServerConnection& connection,
    uint32_t initialRequestN) {
  auto [it, inserted] = streams_.try_emplace(streamId);
  if (!inserted) {
    return nullptr;
  }

  auto cb = std::make_unique<RocketStreamClientCallback>(
      streamId,
      connection,
      initialRequestN,
      THRIFT_FLAG(thrift_enable_stream_counters)
          ? streamMetricCallback_
          : getNoopStreamMetricCallback());

  auto cbPtr = cb.get();
  it->second = std::move(cb);
  return cbPtr;
}

RocketSinkClientCallback* FOLLY_NULLABLE
RefactoredRocketServerConnection::createSinkClientCallback(
    StreamId streamId, IRocketServerConnection& connection) {
  auto [it, inserted] = streams_.try_emplace(streamId);
  if (!inserted) {
    return nullptr;
  }
  auto cb = std::make_unique<RocketSinkClientCallback>(streamId, connection);
  auto cbPtr = cb.get();
  it->second = std::move(cb);
  return cbPtr;
}

std::optional<ChannelRequestCallbackFactory>
RefactoredRocketServerConnection::createChannelClientCallback(
    StreamId streamId,
    IRocketServerConnection& connection,
    uint32_t initialRequestN) {
  auto [it, inserted] = streams_.try_emplace(streamId);
  if (!inserted) {
    return std::nullopt;
  }

  it->second = std::unique_ptr<RocketSinkClientCallback>();
  return ChannelRequestCallbackFactory{
      it->second, streamId, connection, initialRequestN};
}

void RefactoredRocketServerConnection::flushWrites(
    std::unique_ptr<folly::IOBuf> writes, WriteBatchContext&& context) {
  DestructorGuard dg(this);
  DVLOG(10) << fmt::format("write: {} B", writes->computeChainDataLength());

  inflightWritesQueue_.push_back(std::move(context));
  socket_->writeChain(&writerCallback_, std::move(writes));
}

void RefactoredRocketServerConnection::flushWritesWithFds(
    std::unique_ptr<folly::IOBuf> writes,
    WriteBatchContext&& context,
    FdsAndOffsets&& fdsAndOffsets) {
  DestructorGuard dg(this);
  DVLOG(10) << fmt::format(
      "write: {} B + {} FD sets",
      writes->computeChainDataLength(),
      fdsAndOffsets.size());

  // If some of the batched writes had FDs, we need to unbatch them, so that
  // each set of FDs is set with the corresponding data.
  //
  // Future: We batch and then unbatch to avoid the potential performance
  // risk of having the "fast path" of FD-free code track a
  // `vector<unique_ptr<IOBuf>>` instead of a single IOBuf chain.  However,
  // it could be worth the simpler code to try and benchmark such a
  // vector-based "last-minute batching" aggregation as in
  // `RocketClient::writeScheduledRequestsToSocket`.
  IOBufQueue writesQ;
  writesQ.append(std::move(writes)); // we'll `split()` chunks from the front
  size_t prevOffset = 0; // the current start of `writesQ`
  for (size_t i = 0; i < fdsAndOffsets.size(); ++i) {
    DCHECK(!writesQ.empty());
    auto& [fds, fdsOffset] = fdsAndOffsets[i];
    // `GT` as opposed to `GE` because in `enqueueWrite`, `offset` includes
    // `data.length()`, and we don't associate FDs with "empty data".
    CHECK_GT(fdsOffset, prevOffset);

    // After this split, `write` just has `[prevOffset, fdsOffset)`, and we
    // can write it out together with `fds`.
    auto write = writesQ.split(fdsOffset - prevOffset);
    DCHECK_EQ(fdsOffset - prevOffset, write->computeChainDataLength());

    if (writesQ.empty()) {
      // Our writeSuccess / writeError handlers require that every write to
      // the socket be preceded by a matched queue entry.  To avoid complex
      // unbatching of `WriteBatchContext`s in `WriteBatcher::enqueueWrite`,
      // let's make the first N-1 queue entries empty dummies, and use the
      // full batched context for the last write.
      inflightWritesQueue_.push_back(std::move(context));
      // KEEP THIS INVARIANT: For the receiver to correctly match FDs to a
      // message, the FDs must be sent with the IOBuf ending with the FINAL
      // fragment of that message.  Today, message fragments are not
      // interleaved, so there is no explicit logic around this, but this
      // invariant must be preserved going forward.
      writeChainWithFds(
          socket_.get(), &writerCallback_, std::move(write), std::move(fds));
      // Return here so clang-tidy linter won't think the context is moved twice
      return;
    } else {
      inflightWritesQueue_.push_back(WriteBatchContext{});
      writeChainWithFds(
          socket_.get(), &writerCallback_, std::move(write), std::move(fds));
    }

    prevOffset = fdsOffset;
  }
  // This tail segment of data had no FDs attached.
  if (!writesQ.empty()) {
    inflightWritesQueue_.push_back(std::move(context));
    socket_->writeChain(&writerCallback_, writesQ.move());
  }
}

void RefactoredRocketServerConnection::send(
    std::unique_ptr<folly::IOBuf> data,
    MessageChannel::SendCallbackPtr cb,
    StreamId streamId,
    folly::SocketFds fds) {
  writeBatcher_.enqueueWrite(
      std::move(data), std::move(cb), streamId, std::move(fds));
}

void RefactoredRocketServerConnection::sendErrorAfterDrain(
    StreamId streamId, RocketException&& rex) {
  evb_.dcheckIsInEventBaseThread();
  DCHECK(
      state_ == ConnectionState::CLOSING || state_ == ConnectionState::CLOSED);

  writeBatcher_.enqueueWrite(
      ErrorFrame(streamId, std::move(rex)).serialize(),
      nullptr,
      streamId,
      folly::SocketFds{});
}

RefactoredRocketServerConnection::~RefactoredRocketServerConnection() {
  DCHECK(inflightRequests_ == 0);
  DCHECK(inflightWritesQueue_.empty());
  DCHECK(inflightSinkFinalResponses_ == 0);
  DCHECK(writeBatcher_.empty());
  DCHECK(activePausedHandlers_ == 0);
  if (rawSocket_) {
    rawSocket_->setBufferCallback(nullptr);
  }

  // Subtle: Close the socket, which will fail all outstanding writes and
  // unsubscribe the read callback, but do not destroy the object itself, since
  // other member variables of RocketServerConnection may be borrowing the
  // object.
  socket_->closeNow();

  // reclaim any memory in use by pending writes
  if (egressBufferSize_) {
    egressMemoryTracker_.decrement(egressBufferSize_);
    DVLOG(10) << "buffered: 0 (-" << egressBufferSize_ << ") B";
    egressBufferSize_ = 0;
  }
}

namespace {
StreamRpcError getStreamConnectionClosingError() {
  StreamRpcError streamRpcError;
  streamRpcError.code() = StreamRpcErrorCode::SERVER_CLOSING_CONNECTION;
  streamRpcError.name_utf8() =
      apache::thrift::TEnumTraits<StreamRpcErrorCode>::findName(
          StreamRpcErrorCode::SERVER_CLOSING_CONNECTION);
  streamRpcError.what_utf8() = "Server closing connection, cancelling stream";
  return streamRpcError;
}
} // namespace

void RefactoredRocketServerConnection::closeIfNeeded() {
  if (state_ == ConnectionState::DRAINING && inflightRequests_ == 0 &&
      inflightSinkFinalResponses_ == 0) {
    DestructorGuard dg(this);

    socketDrainer_.activate();

    if (drainCompleteCode_) {
      ServerPushMetadata serverMeta;
      serverMeta.drainCompletePush().ensure().drainCompleteCode().from_optional(
          drainCompleteCode_);
      sendMetadataPush(getPayloadSerializer()->packCompact(serverMeta));
      // Send CONNECTION_ERROR error in case client doesn't support
      // DrainCompletePush
      sendError(StreamId{0}, RocketException(ErrorCode::CONNECTION_ERROR));
    }

    state_ = ConnectionState::CLOSING;
    frameHandler_->connectionClosing();
    closeIfNeeded();
    return;
  }

  if (state_ != ConnectionState::CLOSING) {
    return;
  }

  if (!socket_->good()) {
    socketDrainer_.drainComplete();
  }
  if (isBusy() || !socketDrainer_.isDrainComplete()) {
    return;
  }

  DestructorGuard dg(this);
  // Update state_ early, as subsequent lines may call recursively into
  // closeIfNeeded(). Such recursive calls should be no-ops.
  state_ = ConnectionState::CLOSED;

  if (auto* manager = getConnectionManager()) {
    manager->removeConnection(this);
  }

  while (!streams_.empty()) {
    auto callback = std::move(streams_.begin()->second);
    streams_.erase(streams_.begin());
    // Calling application callback may trigger rehashing.
    folly::variant_match(
        callback,
        [&](const std::unique_ptr<RocketStreamClientCallback>& callback) {
          sendErrorAfterDrain(
              callback->streamId(),
              RocketException(
                  ErrorCode::CANCELED,
                  getPayloadSerializer()->packCompact(
                      getStreamConnectionClosingError())));
          callback->onStreamCancel();
        },
        [](const std::unique_ptr<RocketSinkClientCallback>& callback) {
          bool state = callback->onSinkError(TApplicationException(
              TApplicationException::TApplicationExceptionType::INTERRUPTION));
          DCHECK(state) << "onSinkError called after sink complete!";
        },
        [](const std::unique_ptr<RocketBiDiClientCallback>& callback) {
          if (callback->isSinkOpen()) {
            // NOTE that we can receive connection closure AFTER we already
            // cancelled the sink
            callback->onSinkError(TApplicationException(
                TApplicationException::TApplicationExceptionType::
                    INTERRUPTION));
          }
          if (callback->isStreamOpen()) {
            // NOTE that we can receive connection closure AFTER we already
            // cancelled the sink
            callback->onStreamCancel();
          }
        });
    requestComplete();
  }

  writeBatcher_.drain();

  destroy();
}

void RefactoredRocketServerConnection::tryResumeSocketReading() {
  if (state_ == ConnectionState::ALIVE || state_ == ConnectionState::DRAINING) {
    socket_->setReadCB(&parser_);
  }
}

void RefactoredRocketServerConnection::pauseSocketReading() {
  socket_->setReadCB(nullptr);
}

void RefactoredRocketServerConnection::handleFrame(
    std::unique_ptr<folly::IOBuf> frame) {
  DestructorGuard dg(this);
  if (state_ != ConnectionState::ALIVE && state_ != ConnectionState::DRAINING) {
    return;
  }

  frameHandler_->onBeforeHandleFrame();

  // Fast path: use cached pointer to avoid EventBaseLocal lookup overhead
  auto* batcher = cachedBatcher_;
  if (FOLLY_UNLIKELY(batcher == nullptr)) {
    batcher = batcher_.get(evb_);
    if (FOLLY_UNLIKELY(batcher == nullptr)) {
      batcher_.emplace(evb_, evb_);
      batcher = batcher_.get(evb_);
    }
    // Cache the pointer for subsequent calls
    cachedBatcher_ = batcher;
  }

  batcher->handle(std::move(frame), connectionAdapter_, incomingFrameHandler_);
}

void RefactoredRocketServerConnection::handleStreamFrame(
    std::unique_ptr<folly::IOBuf>,
    StreamId,
    FrameType,
    Flags,
    folly::io::Cursor,
    RocketStreamClientCallback&) {}

void RefactoredRocketServerConnection::handleSinkFrame(
    std::unique_ptr<folly::IOBuf> frame,
    StreamId streamId,
    FrameType frameType,
    Flags flags,
    folly::io::Cursor cursor,
    RocketSinkClientCallback& clientCallback) {
  if (!clientCallback.serverCallbackReady()) {
    if (frameType == FrameType::ERROR) {
      ErrorFrame errorFrame{std::move(frame)};
      if (errorFrame.errorCode() == ErrorCode::CANCELED) {
        return clientCallback.earlyCancelled();
      }
    }
    return close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId),
                static_cast<uint8_t>(frameType))));
  }

  auto handleSinkPayload = [&](PayloadFrame&& payloadFrame) {
    const bool next = payloadFrame.hasNext();
    const bool complete = payloadFrame.hasComplete();
    if (auto fullPayload = bufferOrGetFullPayload(std::move(payloadFrame))) {
      bool notViolateContract = true;
      if (next) {
        auto streamPayload = getPayloadSerializer()->unpack<StreamPayload>(
            std::move(*fullPayload), decodeMetadataUsingBinary_.value());
        if (streamPayload.hasException()) {
          notViolateContract =
              clientCallback.onSinkError(std::move(streamPayload.exception()));
          if (notViolateContract) {
            freeStream(streamId, true);
          }
        } else {
          // FIXME: As noted in `RocketClient::handleSinkResponse`, sinks
          // currently lack the codegen to be able to support FD passing.
          DCHECK(
              !streamPayload->metadata.fdMetadata().has_value() ||
              streamPayload->metadata.fdMetadata()->numFds().value_or(0) == 0)
              << "FD passing is not implemented for sinks";

          auto payloadMetadataRef = streamPayload->metadata.payloadMetadata();
          if (payloadMetadataRef &&
              payloadMetadataRef->getType() ==
                  PayloadMetadata::Type::exceptionMetadata) {
            notViolateContract = clientCallback.onSinkError(
                apache::thrift::detail::EncodedStreamError(
                    std::move(streamPayload.value())));
            if (notViolateContract) {
              freeStream(streamId, true);
            }
          } else {
            notViolateContract =
                clientCallback.onSinkNext(std::move(*streamPayload));
          }
        }
      }

      if (complete) {
        // it is possible final response(error) sent from serverCallback,
        // serverCallback may be already destroyed.
        if (streams_.find(streamId) != streams_.end()) {
          notViolateContract = clientCallback.onSinkComplete();
        }
      }

      if (!notViolateContract) {
        close(
            folly::make_exception_wrapper<transport::TTransportException>(
                transport::TTransportException::TTransportExceptionType::
                    STREAMING_CONTRACT_VIOLATION,
                "receiving sink payload frame after sink completion"));
      }
    }
  };

  switch (frameType) {
    case FrameType::PAYLOAD: {
      PayloadFrame payloadFrame(streamId, flags, cursor, std::move(frame));
      handleSinkPayload(std::move(payloadFrame));
    } break;

    case FrameType::ERROR: {
      ErrorFrame errorFrame{std::move(frame)};
      auto ew = [&] {
        if (errorFrame.errorCode() == ErrorCode::CANCELED) {
          return folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::TApplicationExceptionType::INTERRUPTION);
        } else {
          return folly::make_exception_wrapper<RocketException>(
              errorFrame.errorCode(), std::move(errorFrame.payload()).data());
        }
      }();

      bool notViolateContract = clientCallback.onSinkError(std::move(ew));
      if (notViolateContract) {
        freeStream(streamId, true);
      } else {
        close(
            folly::make_exception_wrapper<transport::TTransportException>(
                transport::TTransportException::TTransportExceptionType::
                    STREAMING_CONTRACT_VIOLATION,
                "receiving sink error frame after sink completion"));
      }
    } break;

    case FrameType::EXT: {
      ExtFrame extFrame(streamId, flags, cursor, std::move(frame));
      extFrame.extFrameType();
    }
      [[fallthrough]];

    default:
      close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID,
              fmt::format(
                  "Received unhandleable frame type ({}) for sink (id {})",
                  static_cast<uint8_t>(frameType),
                  static_cast<uint32_t>(streamId))));
  }
}

void RefactoredRocketServerConnection::handleBiDiFrame(
    std::unique_ptr<folly::IOBuf> frame,
    StreamId streamId,
    FrameType frameType,
    Flags flags,
    folly::io::Cursor cursor,
    RocketBiDiClientCallback& clientCallback) {
  if (!clientCallback.serverCallbackReady()) {
    // early cancellation
    if (frameType == FrameType::ERROR) {
      ErrorFrame errorFrame{std::move(frame)};
      if (errorFrame.errorCode() == ErrorCode::CANCELED) {
        clientCallback.cancelEarly();
        return;
      }
    }
    // unexpected frame
    close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId),
                static_cast<uint8_t>(frameType))));
    return;
  }

  auto getErrorFromPayload = [](folly::Try<StreamPayload>& payload)
      -> std::optional<folly::exception_wrapper> {
    if (payload.hasException()) {
      return payload.exception();
    }

    if (auto metadata = payload->metadata.payloadMetadata()) {
      if (metadata->getType() == PayloadMetadata::Type::exceptionMetadata) {
        return apache::thrift::detail::EncodedStreamError(std::move(*payload));
      }
    }

    return std::nullopt;
  };

  auto dcheckFDPassingIsNotSupported = [](folly::Try<StreamPayload>& payload) {
    // As noted in `RocketClient::handleSinkResponse`, bidi streams
    // currently lack the codegen to be able to support FD passing.
    DCHECK(
        !payload->metadata.fdMetadata().has_value() ||
        payload->metadata.fdMetadata()->numFds().value_or(0) == 0)
        << "FD passing is not implemented for bidi streams";
  };

  auto handlePayloadFrame = [&](PayloadFrame&& payloadFrame) {
    const bool next = payloadFrame.hasNext();
    const bool complete = payloadFrame.hasComplete();
    bool alive = true;

    if (auto fullPayload = bufferOrGetFullPayload(std::move(payloadFrame))) {
      if (next) {
        auto streamPayload = getPayloadSerializer()->unpack<StreamPayload>(
            std::move(*fullPayload), decodeMetadataUsingBinary_.value());

        dcheckFDPassingIsNotSupported(streamPayload);

        if (auto error = getErrorFromPayload(streamPayload)) {
          if (clientCallback.isSinkOpen()) {
            alive = clientCallback.onSinkError(std::move(*error));
          }
        } else {
          if (clientCallback.isSinkOpen()) {
            alive = clientCallback.onSinkNext(std::move(*streamPayload));
          }
        }
      }

      if (complete) {
        if (alive && clientCallback.isSinkOpen()) {
          std::ignore = clientCallback.onSinkComplete();
        }
      }
    }
  };

  auto handleErrorFrame = [&](ErrorFrame&& errorFrame) {
    auto ew = [&] {
      if (errorFrame.errorCode() == ErrorCode::CANCELED) {
        return folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::INTERRUPTION);
      } else {
        return folly::make_exception_wrapper<RocketException>(
            errorFrame.errorCode(), std::move(errorFrame.payload()).data());
      }
    }();

    if (clientCallback.isSinkOpen()) {
      std::ignore = clientCallback.onSinkError(std::move(ew));
    }
  };

  auto handleRequestNFrame = [&](RequestNFrame&& requestNFrame) {
    if (clientCallback.isStreamOpen()) {
      clientCallback.onStreamRequestN(requestNFrame.requestN());
    }
  };

  auto handleCancelFrame = [&]() { clientCallback.onStreamCancel(); };

  switch (frameType) {
    case FrameType::PAYLOAD: {
      handlePayloadFrame(
          PayloadFrame{streamId, flags, cursor, std::move(frame)});
    } break;
    case FrameType::ERROR: {
      handleErrorFrame(ErrorFrame(std::move(frame)));
    } break;
    case FrameType::REQUEST_N: {
      handleRequestNFrame(RequestNFrame{streamId, flags, cursor});
    } break;
    case FrameType::CANCEL: {
      handleCancelFrame();
    } break;
    case FrameType::EXT: {
      ExtFrame extFrame(streamId, flags, cursor, std::move(frame));
      if (!extFrame.hasIgnore()) {
        close(
            folly::make_exception_wrapper<RocketException>(
                ErrorCode::INVALID,
                fmt::format(
                    "Received unsupported EXT frame type ({}) for stream (id {})",
                    static_cast<uint32_t>(extFrame.extFrameType()),
                    static_cast<uint32_t>(streamId))));
      }
    } break;
    default: {
      close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID,
              fmt::format(
                  "Received unsupported frame type ({}) for bidi stream (id {})",
                  static_cast<uint8_t>(frameType),
                  static_cast<uint32_t>(streamId))));
    }
  }
}

void RefactoredRocketServerConnection::close(folly::exception_wrapper ew) {
  if (state_ == ConnectionState::CLOSING || state_ == ConnectionState::CLOSED) {
    closeIfNeeded();
    return;
  }

  DestructorGuard dg(this);

  socketDrainer_.activate();

  if (!socket_->error()) {
    if (!ew.with_exception<RocketException>([this](RocketException rex) {
          sendError(StreamId{0}, std::move(rex));
        })) {
      auto rex = ew
          ? RocketException(ErrorCode::CONNECTION_ERROR, ew.what())
          : RocketException(ErrorCode::CONNECTION_CLOSE, "Closing connection");
      sendError(StreamId{0}, std::move(rex));
    }
  }

  state_ = ConnectionState::CLOSING;
  frameHandler_->connectionClosing();
  closeIfNeeded();
}

void RefactoredRocketServerConnection::timeoutExpired() noexcept {
  DestructorGuard dg(this);

  if (!isBusy()) {
    closeWhenIdle();
  }
}

bool RefactoredRocketServerConnection::isBusy() const {
  return inflightRequests_ != 0 || !inflightWritesQueue_.empty() ||
      inflightSinkFinalResponses_ != 0 || !writeBatcher_.empty() ||
      activePausedHandlers_ != 0 || isOutgoingFrameHandlerBusy();
}

// Helper method to check if OutgoingFrameHandler has pending frames
bool RefactoredRocketServerConnection::isOutgoingFrameHandlerBusy() const {
  if (THRIFT_FLAG(rocket_use_outgoing_frame_handler)) {
    // Note: We can't call get() on const EventBaseLocal, so we use a
    // conservative approach for now
    // TODO: Enhance OutgoingFrameHandler to expose busy state in future phases
    // For now, assume not busy since we can't check the actual state
    return false; // Conservative approach - assume not busy for now
  }
  return false;
}

// On graceful shutdown, ConnectionManager will first fire the
// notifyPendingShutdown() callback for each connection. Then, after the drain
// period has elapsed, closeWhenIdle() will be called for each connection.
// Note that ConnectionManager waits for a connection to become un-busy before
// calling closeWhenIdle().
void RefactoredRocketServerConnection::notifyPendingShutdown() {
  startDrain({});
}

void RefactoredRocketServerConnection::startDrain(
    std::optional<DrainCompleteCode> drainCompleteCode) {
  if (state_ != ConnectionState::ALIVE) {
    return;
  }

  state_ = ConnectionState::DRAINING;
  drainCompleteCode_ = drainCompleteCode;
  sendError(StreamId{0}, RocketException(ErrorCode::CONNECTION_CLOSE));
  closeIfNeeded();
}

void RefactoredRocketServerConnection::dropConnection(
    const std::string& errorMsg) {
  DestructorGuard dg(this);

  // Subtle: skip the socket draining process and stop new reads
  socketDrainer_.drainComplete();
  // Subtle: flush pending writes to ensure isBusy() returns false
  writeBatcher_.drain();
  // Subtle: preemptively close socket to fail all outstanding writes (also
  // needed to ensure isBusy() returns false)
  socket_->closeNow();

  if (!errorMsg.empty()) {
    if (auto context = frameHandler_->getCpp2ConnContext()) {
      THRIFT_CONNECTION_EVENT(drop_connection.rocket)
          .log(*context, [&errorMsg]() {
            return folly::dynamic::object("error", errorMsg);
          });
    }
  }

  close(
      folly::make_exception_wrapper<transport::TTransportException>(
          transport::TTransportException::TTransportExceptionType::INTERRUPTED,
          "Dropping connection"));
}

void RefactoredRocketServerConnection::closeWhenIdle() {
  socketDrainer_.drainComplete();
  close(
      folly::make_exception_wrapper<transport::TTransportException>(
          transport::TTransportException::TTransportExceptionType::INTERRUPTED,
          "Closing due to imminent shutdown"));
}

void RefactoredRocketServerConnection::scheduleStreamTimeout(
    folly::HHWheelTimer::Callback* timeoutCallback) {
  // Delegate to StreamCallbackManager - STREAMING-ONLY refactoring
  streamCallbackManager_.scheduleStreamTimeout(timeoutCallback);
}

void RefactoredRocketServerConnection::scheduleSinkTimeout(
    folly::HHWheelTimer::Callback* timeoutCallback,
    std::chrono::milliseconds timeout) {
  if (timeout != std::chrono::milliseconds::zero()) {
    evb_.timer().scheduleTimeout(timeoutCallback, timeout);
  }
}

folly::Optional<Payload>
RefactoredRocketServerConnection::bufferOrGetFullPayload(
    PayloadFrame&& payloadFrame) {
  folly::Optional<Payload> fullPayload;

  const auto streamId = payloadFrame.streamId();
  const bool hasFollows = payloadFrame.hasFollows();
  const auto it = bufferedFragments_.find(streamId);

  if (hasFollows) {
    if (it != bufferedFragments_.end()) {
      auto& firstFragments = it->second;
      firstFragments.append(std::move(payloadFrame.payload()));
    } else {
      bufferedFragments_.emplace(streamId, std::move(payloadFrame.payload()));
    }
  } else {
    if (it != bufferedFragments_.end()) {
      auto firstFragments = std::move(it->second);
      bufferedFragments_.erase(it);
      firstFragments.append(std::move(payloadFrame.payload()));
      fullPayload = std::move(firstFragments);
    } else {
      fullPayload = std::move(payloadFrame.payload());
    }
  }

  return fullPayload;
}

void RefactoredRocketServerConnection::sendPayload(
    StreamId streamId,
    Payload&& payload,
    Flags flags,
    apache::thrift::MessageChannel::SendCallbackPtr cb) {
  if (THRIFT_FLAG(rocket_use_outgoing_frame_handler)) {
    // New path: Use OutgoingFrameHandler for frame-level batching
    evb_.dcheckIsInEventBaseThread();
    if (state_ != ConnectionState::ALIVE &&
        state_ != ConnectionState::DRAINING) {
      return;
    }

    // Get or create OutgoingFrameHandler for this EventBase
    auto* handler = outgoingFrameHandler_.get(evb_);
    if (FOLLY_UNLIKELY(handler == nullptr)) {
      outgoingFrameHandler_.emplace(
          evb_, evb_, cfg_.getOutgoingFrameHandlerBatchLogSize());
      handler = outgoingFrameHandler_.get(evb_);
    }

    handler->handle(
        PayloadFrame(streamId, std::move(payload), flags), connectionAdapter_);

  } else {
    // Existing path: Use WriteBatcher directly
    auto fds = std::move(payload.fds);
    send(
        PayloadFrame(streamId, std::move(payload), flags).serialize(),
        std::move(cb),
        streamId,
        std::move(fds));
  }
}

void RefactoredRocketServerConnection::sendError(
    StreamId streamId,
    RocketException&& rex,
    apache::thrift::MessageChannel::SendCallbackPtr cb) {
  if (THRIFT_FLAG(rocket_use_outgoing_frame_handler)) {
    // New path: Use OutgoingFrameHandler for frame-level batching
    evb_.dcheckIsInEventBaseThread();
    if (state_ != ConnectionState::ALIVE &&
        state_ != ConnectionState::DRAINING) {
      return;
    }

    // Get or create OutgoingFrameHandler for this EventBase
    auto* handler = outgoingFrameHandler_.get(evb_);
    if (FOLLY_UNLIKELY(handler == nullptr)) {
      outgoingFrameHandler_.emplace(
          evb_, evb_, cfg_.getOutgoingFrameHandlerBatchLogSize());
      handler = outgoingFrameHandler_.get(evb_);
    }

    // Use OutgoingFrameHandler for non-FD frames (ErrorFrame has no FDs)
    handler->handle(
        ErrorFrame(streamId, std::move(rex)),
        connectionAdapter_,
        std::move(cb));
  } else {
    // Existing path: Use WriteBatcher directly
    send(
        ErrorFrame(streamId, std::move(rex)).serialize(),
        std::move(cb),
        streamId);
  }
}

void RefactoredRocketServerConnection::sendRequestN(
    StreamId streamId, int32_t n) {
  if (THRIFT_FLAG(rocket_use_outgoing_frame_handler)) {
    // New path: Use OutgoingFrameHandler for frame-level batching
    evb_.dcheckIsInEventBaseThread();
    if (state_ != ConnectionState::ALIVE &&
        state_ != ConnectionState::DRAINING) {
      return;
    }

    // Get or create OutgoingFrameHandler for this EventBase
    auto* handler = outgoingFrameHandler_.get(evb_);
    if (FOLLY_UNLIKELY(handler == nullptr)) {
      outgoingFrameHandler_.emplace(
          evb_, evb_, cfg_.getOutgoingFrameHandlerBatchLogSize());
      handler = outgoingFrameHandler_.get(evb_);
    }

    // Use OutgoingFrameHandler for non-FD frames (RequestNFrame has no FDs)
    handler->handle(RequestNFrame(streamId, n), connectionAdapter_, nullptr);
  } else {
    // Existing path: Use WriteBatcher directly
    send(RequestNFrame(streamId, n).serialize(), nullptr, streamId);
  }
}

void RefactoredRocketServerConnection::sendCancel(StreamId streamId) {
  if (THRIFT_FLAG(rocket_use_outgoing_frame_handler)) {
    // New path: Use OutgoingFrameHandler for frame-level batching
    evb_.dcheckIsInEventBaseThread();
    if (state_ != ConnectionState::ALIVE &&
        state_ != ConnectionState::DRAINING) {
      return;
    }

    // Get or create OutgoingFrameHandler for this EventBase
    auto* handler = outgoingFrameHandler_.get(evb_);
    if (FOLLY_UNLIKELY(handler == nullptr)) {
      outgoingFrameHandler_.emplace(
          evb_, evb_, cfg_.getOutgoingFrameHandlerBatchLogSize());
      handler = outgoingFrameHandler_.get(evb_);
    }

    // Use OutgoingFrameHandler for non-FD frames (CancelFrame has no FDs)
    handler->handle(CancelFrame(streamId), connectionAdapter_, nullptr);
  } else {
    // Existing path: Use WriteBatcher directly
    send(CancelFrame(streamId).serialize(), nullptr, streamId);
  }
}

void RefactoredRocketServerConnection::sendMetadataPush(
    std::unique_ptr<folly::IOBuf> metadata) {
  if (THRIFT_FLAG(rocket_use_outgoing_frame_handler)) {
    // New path: Use OutgoingFrameHandler for frame-level batching
    evb_.dcheckIsInEventBaseThread();
    if (state_ != ConnectionState::ALIVE &&
        state_ != ConnectionState::DRAINING) {
      return;
    }

    // Get or create OutgoingFrameHandler for this EventBase
    auto* handler = outgoingFrameHandler_.get(evb_);
    if (FOLLY_UNLIKELY(handler == nullptr)) {
      outgoingFrameHandler_.emplace(
          evb_, evb_, cfg_.getOutgoingFrameHandlerBatchLogSize());
      handler = outgoingFrameHandler_.get(evb_);
    }

    // Use OutgoingFrameHandler for non-FD frames (MetadataPushFrame has no FDs)
    handler->handle(
        MetadataPushFrame::makeFromMetadata(std::move(metadata)),
        connectionAdapter_,
        nullptr);
  } else {
    // Existing path: Use WriteBatcher directly
    send(MetadataPushFrame::makeFromMetadata(std::move(metadata)).serialize());
  }
}

void RefactoredRocketServerConnection::freeStream(
    StreamId streamId, bool markRequestComplete) {
  DestructorGuard dg(this);

  bufferedFragments_.erase(streamId);

  DCHECK(streams_.find(streamId) != streams_.end());
  streams_.erase(streamId);
  if (markRequestComplete) {
    requestComplete();
  }

  // Delegate to StreamCallbackManager - STREAMING-ONLY refactoring
  streamCallbackManager_.freeStream(streamId, markRequestComplete);
}

void RefactoredRocketServerConnection::applyQosMarking(
    const RequestSetupMetadata& setupMetadata) {
  constexpr int32_t kMaxDscpValue = (1 << 6) - 1;

  if (!rawSocket_) {
    return;
  }

  try {
    folly::SocketAddress addr;
    rawSocket_->getAddress(&addr);
    if (addr.getFamily() != AF_INET6 && addr.getFamily() != AF_INET) {
      return;
    }

    const auto fd = rawSocket_->getNetworkSocket();

    if (auto dscp = setupMetadata.dscpToReflect()) {
      if (auto context = frameHandler_->getCpp2ConnContext()) {
        THRIFT_CONNECTION_EVENT(rocket.dscp).log(*context, [&] {
          return folly::dynamic::object("rocket_dscp", *dscp);
        });
      }
      if (*dscp >= 0 && *dscp <= kMaxDscpValue) {
        const folly::SocketOptionKey kIpv4TosKey = {IPPROTO_IP, IP_TOS};
        const folly::SocketOptionKey kIpv6TosKey = {IPPROTO_IPV6, IPV6_TCLASS};
        auto& dscpKey = addr.getIPAddress().isV4() ? kIpv4TosKey : kIpv6TosKey;
        dscpKey.apply(fd, *dscp << 2);
      }
    }
#if defined(SO_MARK)
    if (auto mark = setupMetadata.markToReflect()) {
      const folly::SocketOptionKey kSoMarkKey = {SOL_SOCKET, SO_MARK};
      kSoMarkKey.apply(fd, *mark);
    }
#endif

    plugin::applyCustomQosMarkingToSocket(fd, setupMetadata);
  } catch (const std::exception& ex) {
    FB_LOG_EVERY_MS(WARNING, 60 * 1000)
        << "Failed to apply DSCP to socket: " << folly::exceptionStr(ex);
  } catch (...) {
    FB_LOG_EVERY_MS(WARNING, 60 * 1000)
        << "Failed to apply DSCP to socket: "
        << folly::exceptionStr(folly::current_exception());
  }
}

void RefactoredRocketServerConnection::pauseStreams() {
  // Delegate to StreamCallbackManager - STREAMING-ONLY refactoring
  streamCallbackManager_.pauseStreams();
}

void RefactoredRocketServerConnection::resumeStreams() {
  // Delegate to StreamCallbackManager - STREAMING-ONLY refactoring
  streamCallbackManager_.resumeStreams();
}

bool RefactoredRocketServerConnection::incMemoryUsage(uint32_t memSize) {
  if (!ingressMemoryTracker_.increment(memSize)) {
    ingressMemoryTracker_.decrement(memSize);
    socket_->setReadCB(nullptr);
    startDrain(DrainCompleteCode::EXCEEDED_INGRESS_MEM_LIMIT);
    FB_LOG_EVERY_MS(ERROR, 1000) << fmt::format(
        "Dropping connection for ({}): exceeded ingress memory limit ({}). The config value of min increment size: ({})",
        getPeerAddress().describe(),
        ingressMemoryTracker_.getMemLimit(),
        ingressMemoryTracker_.getMinIncrementSize());
    if (auto context = frameHandler_->getCpp2ConnContext()) {
      THRIFT_CONNECTION_EVENT(exceeded_ingress_mem_limit).log(*context, [&] {
        folly::dynamic metadata = folly::dynamic::object;
        metadata["mem_limit"] = ingressMemoryTracker_.getMemLimit();
        metadata["min_increment_size"] =
            ingressMemoryTracker_.getMinIncrementSize();
        return metadata;
      });
    }
    return false;
  }
  return true;
}

void RefactoredRocketServerConnection::requestComplete() {
  if (!writeBatcher_.empty()) {
    writeBatcher_.enqueueRequestComplete();
    return;
  }
  if (!inflightWritesQueue_.empty()) {
    inflightWritesQueue_.back().requestCompleteCount++;
    return;
  }
  frameHandler_->requestComplete();
}

} // namespace apache::thrift::rocket
