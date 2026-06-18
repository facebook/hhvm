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

#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>

#include <memory>
#include <utility>

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

#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/LoggingEventTransportMetadata.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketBiDiClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionPlugins.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>

THRIFT_FLAG_DEFINE_bool(enable_rocket_connection_observers, false);
THRIFT_FLAG_DEFINE_bool(thrift_enable_stream_counters, true);
THRIFT_FLAG_DEFINE_bool(thrift_check_free_stream_thread, true);

namespace apache::thrift::rocket {

ChannelRequestCallbackFactory::ChannelRequestCallbackFactory(
    ClientCallbackUniquePtr& callback,
    StreamId streamId,
    IRocketServerConnection& connection,
    uint32_t initialRequestN)
    : callback_(&callback),
      streamId_(streamId),
      connection_(&connection),
      initialRequestN_(initialRequestN) {}

template <>
RocketBiDiClientCallback*
ChannelRequestCallbackFactory::create<RocketBiDiClientCallback>() {
  auto ret = std::make_unique<RocketBiDiClientCallback>(
      streamId_,
      static_cast<RocketServerConnection&>(*connection_),
      initialRequestN_);
  auto ptr = ret.get();
  *callback_ = std::move(ret);
  return ptr;
}

template <>
RocketSinkClientCallback*
ChannelRequestCallbackFactory::create<RocketSinkClientCallback>() {
  auto ret = std::make_unique<RocketSinkClientCallback>(
      streamId_, static_cast<RocketServerConnection&>(*connection_));
  auto ptr = ret.get();
  *callback_ = std::move(ret);
  return ptr;
}

RocketServerConnection::RocketServerConnection(
    folly::AsyncTransport::UniquePtr socket,
    std::unique_ptr<RocketServerHandler> frameHandler,
    MemoryTracker& ingressMemoryTracker,
    MemoryTracker& egressMemoryTracker,
    const Config& cfg)
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
      writeBatcher_(
          *this,
          cfg.writeBatchingInterval,
          cfg.writeBatchingSize,
          cfg.writeBatchingByteSize),
      socketDrainer_(*this),
      ingressMemoryTracker_(ingressMemoryTracker),
      egressMemoryTracker_(egressMemoryTracker) {
  CHECK(socket_);
  CHECK(frameHandler_);

  peerAddress_ = socket_->getPeerAddress();

  socket_->setReadCB(&parser_);
  if (rawSocket_) {
    rawSocket_->setBufferCallback(this);
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

RocketStreamClientCallback* FOLLY_NULLABLE
RocketServerConnection::createStreamClientCallback(
    StreamId streamId,
    IRocketServerConnection& connection,
    uint32_t initialRequestN) {
  auto [it, inserted] = streams_.try_emplace(streamId);
  if (!inserted) {
    return nullptr;
  }
  auto cb = std::make_unique<RocketStreamClientCallback>(
      streamId,
      static_cast<RocketServerConnection&>(connection),
      initialRequestN);
  auto cbPtr = cb.get();
  it->second = std::move(cb);
  return cbPtr;
}

RocketSinkClientCallback* FOLLY_NULLABLE
RocketServerConnection::createSinkClientCallback(
    StreamId streamId, IRocketServerConnection& connection) {
  auto [it, inserted] = streams_.try_emplace(streamId);
  if (!inserted) {
    return nullptr;
  }
  auto cb = std::make_unique<RocketSinkClientCallback>(
      streamId, static_cast<RocketServerConnection&>(connection));
  auto cbPtr = cb.get();
  it->second = std::move(cb);
  return cbPtr;
}

std::optional<ChannelRequestCallbackFactory>
RocketServerConnection::createChannelClientCallback(
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

void RocketServerConnection::flushWrites(
    std::unique_ptr<folly::IOBuf> writes, WriteBatchContext&& context) {
  DestructorGuard dg(this);
  DVLOG(10) << fmt::format("write: {} B", writes->computeChainDataLength());

  inflightWritesQueue_.push_back(std::move(context));
  socket_->writeChain(this, std::move(writes));
}

void RocketServerConnection::flushWritesWithFds(
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
      writeChainWithFds(socket_.get(), this, std::move(write), std::move(fds));
      // Return here so clang-tidy linter won't think the context is moved twice
      return;
    } else {
      inflightWritesQueue_.push_back(WriteBatchContext{});
      writeChainWithFds(socket_.get(), this, std::move(write), std::move(fds));
    }

    prevOffset = fdsOffset;
  }
  // This tail segment of data had no FDs attached.
  if (!writesQ.empty()) {
    inflightWritesQueue_.push_back(std::move(context));
    socket_->writeChain(this, writesQ.move());
  }
}

void RocketServerConnection::send(
    std::unique_ptr<folly::IOBuf> data,
    MessageChannel::SendCallbackPtr cb,
    StreamId streamId,
    folly::SocketFds fds) {
  writeBatcher_.enqueueWrite(
      std::move(data), std::move(cb), streamId, std::move(fds));
}

void RocketServerConnection::sendErrorAfterDrain(
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

RocketServerConnection::~RocketServerConnection() {
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

void RocketServerConnection::closeIfNeeded() {
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
    auto handler = std::move(streams_.begin()->second);
    streams_.erase(streams_.begin());
    // Calling application callback may trigger rehashing.
    handler->handleConnectionClose();
    requestComplete();
  }

  writeBatcher_.drain();

  destroy();
}

void RocketServerConnection::incrementActivePauseHandlers() {
  ++activePausedHandlers_;
}

void RocketServerConnection::decrementActivePauseHandlers() {
  --activePausedHandlers_;
}

void RocketServerConnection::tryResumeSocketReading() {
  if (state_ == ConnectionState::ALIVE || state_ == ConnectionState::DRAINING) {
    socket_->setReadCB(&parser_);
  }
}

void RocketServerConnection::pauseSocketReading() {
  socket_->setReadCB(nullptr);
}

void RocketServerConnection::handleFrame(std::unique_ptr<folly::IOBuf> frame) {
  DestructorGuard dg(this);

  if (state_ != ConnectionState::ALIVE && state_ != ConnectionState::DRAINING) {
    return;
  }

  frameHandler_->onBeforeHandleFrame();

  folly::io::Cursor cursor(frame.get());
  const auto streamId = readStreamId(cursor);
  FrameType frameType;
  Flags flags;
  std::tie(frameType, flags) = readFrameTypeAndFlags(cursor);

  if (UNLIKELY(!setupFrameReceived_)) {
    if (frameType != FrameType::SETUP) {
      return close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID_SETUP, "First frame must be SETUP frame"));
    }
    DCHECK(!decodeMetadataUsingBinary_.has_value());
    setupFrameReceived_ = true;
  } else {
    if (UNLIKELY(frameType == FrameType::SETUP)) {
      return close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID_SETUP, "More than one SETUP frame received"));
    }
    DCHECK(decodeMetadataUsingBinary_.has_value());
  }

  switch (frameType) {
    case FrameType::SETUP: {
      auto setupFrame = SetupFrame(std::move(frame));
      decodeMetadataUsingBinary_.emplace(
          setupFrame.encodeMetadataUsingBinary());
      return frameHandler_->handleSetupFrame(std::move(setupFrame), *this);
    }

    case FrameType::REQUEST_RESPONSE: {
      return handleRequestFrame(
          RequestResponseFrame(streamId, flags, cursor, std::move(frame)));
    }

    case FrameType::REQUEST_FNF: {
      return handleRequestFrame(
          RequestFnfFrame(streamId, flags, cursor, std::move(frame)));
    }

    case FrameType::REQUEST_STREAM: {
      return handleRequestFrame(
          RequestStreamFrame(streamId, flags, cursor, std::move(frame)));
    }

    case FrameType::REQUEST_CHANNEL: {
      return handleRequestFrame(
          RequestChannelFrame(streamId, flags, cursor, std::move(frame)));
    }

    case FrameType::KEEPALIVE: {
      if (streamId == StreamId{0}) {
        KeepAliveFrame keepAliveFrame{std::move(frame)};
        if (keepAliveFrame.hasRespondFlag()) {
          // Echo back data without 'respond' flag
          send(
              KeepAliveFrame{Flags(), std::move(keepAliveFrame).data()}
                  .serialize());
        }
      } else {
        close(
            folly::make_exception_wrapper<RocketException>(
                ErrorCode::CONNECTION_ERROR,
                fmt::format(
                    "Received keepalive frame with non-zero stream ID {}",
                    static_cast<uint32_t>(streamId))));
      }
      return;
    }

    // for the rest of frame types, how to deal with them depends on whether
    // they are part of a streaming or sink (or channel)
    default: {
      auto iter = streams_.find(streamId);
      if (UNLIKELY(iter == streams_.end())) {
        handleUntrackedFrame(
            std::move(frame), streamId, frameType, flags, std::move(cursor));
      } else {
        auto& handler = iter->second;
        switch (frameType) {
          case FrameType::REQUEST_N: {
            handler->handleFrame(RequestNFrame(streamId, flags, cursor));
            break;
          }
          case FrameType::CANCEL: {
            handler->handleFrame(CancelFrame(streamId));
            break;
          }
          case FrameType::PAYLOAD: {
            handler->handleFrame(
                PayloadFrame(streamId, flags, cursor, std::move(frame)));
            break;
          }
          case FrameType::ERROR: {
            handler->handleFrame(ErrorFrame(std::move(frame)));
            break;
          }
          case FrameType::EXT: {
            handler->handleFrame(
                ExtFrame(streamId, flags, cursor, std::move(frame)));
            break;
          }
          default:
            close(
                folly::make_exception_wrapper<RocketException>(
                    ErrorCode::INVALID,
                    fmt::format(
                        "Received unhandleable frame type ({})",
                        static_cast<uint8_t>(frameType))));
        }
      }
    }
  }
}

void RocketServerConnection::handleUntrackedFrame(
    std::unique_ptr<folly::IOBuf> frame,
    StreamId streamId,
    FrameType frameType,
    Flags flags,
    folly::io::Cursor cursor) {
  switch (frameType) {
    case FrameType::PAYLOAD: {
      auto it = partialRequestFrames_.find(streamId);
      if (it == partialRequestFrames_.end()) {
        return;
      }

      PayloadFrame payloadFrame(streamId, flags, cursor, std::move(frame));
      folly::variant_match(it->second, [&](auto& requestFrame) {
        const bool hasFollows = payloadFrame.hasFollows();
        requestFrame.payload().append(std::move(payloadFrame.payload()));
        if (!hasFollows) {
          RocketServerFrameContext(*this, streamId)
              .onFullFrame(std::move(requestFrame));
          partialRequestFrames_.erase(streamId);
        }
      });
      return;
    }
    case FrameType::CANCEL:
      [[fallthrough]];
    case FrameType::REQUEST_N:
      [[fallthrough]];
    case FrameType::ERROR:
      return;

    case FrameType::EXT: {
      ExtFrame extFrame(streamId, flags, cursor, std::move(frame));
      if (!extFrame.hasIgnore()) {
        close(
            folly::make_exception_wrapper<RocketException>(
                ErrorCode::INVALID,
                fmt::format(
                    "Received unhandleable ext frame type ({}) without ignore flag",
                    static_cast<uint32_t>(extFrame.extFrameType()))));
      }
      return;
    }

    case FrameType::METADATA_PUSH: {
      MetadataPushFrame metadataFrame(std::move(frame));
      ClientPushMetadata clientMeta;
      try {
        getPayloadSerializer()->unpack(
            clientMeta, metadataFrame.metadata(), false);
      } catch (...) {
        close(
            folly::make_exception_wrapper<RocketException>(
                ErrorCode::INVALID,
                "Failed to deserialize metadata push frame"));
        return;
      }
      switch (clientMeta.getType()) {
        case ClientPushMetadata::Type::interactionTerminate: {
          frameHandler_->terminateInteraction(
              *clientMeta.interactionTerminate()->interactionId());
          break;
        }
        case ClientPushMetadata::Type::streamHeadersPush: {
          StreamId sid(clientMeta.streamHeadersPush()->streamId().value_or(0));
          auto it = streams_.find(sid);
          if (it != streams_.end()) {
            it->second->handleStreamHeadersPush(
                HeadersPayload(clientMeta.streamHeadersPush()
                                   ->headersPayloadContent()
                                   .value_or({})));
          }
          break;
        }
        case ClientPushMetadata::Type::transportMetadataPush: {
          if (auto context = frameHandler_->getCpp2ConnContext()) {
            auto md = clientMeta.transportMetadataPush()->transportMetadata();
            std::optional<folly::F14NodeMap<std::string, std::string>> metadata;
            if (md) {
              metadata = std::move(*md);
            }
            logTransportMetadata(*context, std::move(metadata));
          }
          break;
        }
        default:
          break;
      }
      return;
    }

    default:
      close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID,
              fmt::format(
                  "Received unhandleable frame type ({})",
                  static_cast<uint8_t>(frameType))));
  }
}

void RocketServerConnection::close(folly::exception_wrapper ew) {
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

void RocketServerConnection::timeoutExpired() noexcept {
  DestructorGuard dg(this);

  if (!isBusy()) {
    frameHandler_->onIdleTimeout();
    closeWhenIdle();
  }
}

bool RocketServerConnection::isBusy() const {
  return inflightRequests_ != 0 || !inflightWritesQueue_.empty() ||
      inflightSinkFinalResponses_ != 0 || !writeBatcher_.empty() ||
      activePausedHandlers_ != 0;
}

// On graceful shutdown, ConnectionManager will first fire the
// notifyPendingShutdown() callback for each connection. Then, after the drain
// period has elapsed, closeWhenIdle() will be called for each connection.
// Note that ConnectionManager waits for a connection to become un-busy before
// calling closeWhenIdle().
void RocketServerConnection::notifyPendingShutdown() {
  startDrain({});
}

void RocketServerConnection::startDrain(
    std::optional<DrainCompleteCode> drainCompleteCode) {
  if (state_ != ConnectionState::ALIVE) {
    return;
  }

  state_ = ConnectionState::DRAINING;
  drainCompleteCode_ = drainCompleteCode;
  sendError(StreamId{0}, RocketException(ErrorCode::CONNECTION_CLOSE));
  closeIfNeeded();
}

void RocketServerConnection::dropConnection(const std::string& errorMsg) {
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

void RocketServerConnection::closeWhenIdle() {
  socketDrainer_.drainComplete();
  close(
      folly::make_exception_wrapper<transport::TTransportException>(
          transport::TTransportException::TTransportExceptionType::INTERRUPTED,
          "Closing due to imminent shutdown"));
}

void RocketServerConnection::writeStarting() noexcept {
  DestructorGuard dg(this);
  DCHECK(!inflightWritesQueue_.empty());
  auto& context = inflightWritesQueue_.front();
  DCHECK(!context.writeEventsContext.startRawByteOffset.has_value());
  context.writeEventsContext.startRawByteOffset = socket_->getRawBytesWritten();

  if (auto observerContainer = getObserverContainer();
      observerContainer && observerContainer->numObservers()) {
    for (const auto& writeEvent : context.writeEvents) {
      observerContainer->invokeInterfaceMethodAllObservers(
          [&](auto observer, auto observed) {
            observer->writeStarting(observed, writeEvent);
          });
    }
  }
}

void RocketServerConnection::writeSuccess() noexcept {
  DestructorGuard dg(this);
  DCHECK(!inflightWritesQueue_.empty());
  auto& context = inflightWritesQueue_.front(); // NB: Can be empty.
  for (auto processingCompleteCount = context.requestCompleteCount;
       processingCompleteCount > 0;
       --processingCompleteCount) {
    frameHandler_->requestComplete();
  }
  DCHECK(!context.writeEventsContext.endRawByteOffset.has_value());
  if (context.writeEventsContext.startRawByteOffset.has_value()) {
    context.writeEventsContext.endRawByteOffset = std::max(
        context.writeEventsContext.startRawByteOffset.value(),
        socket_->getRawBytesWritten() - 1);
  }

  if (auto observerContainer = getObserverContainer();
      observerContainer && observerContainer->numObservers()) {
    for (const auto& writeEvent : context.writeEvents) {
      observerContainer->invokeInterfaceMethodAllObservers(
          [&](auto observer, auto observed) {
            observer->writeSuccess(
                observed, writeEvent, context.writeEventsContext);
          });
    }
  }

  for (auto& cb : context.sendCallbacks) {
    cb.release()->messageSent();
  }

  inflightWritesQueue_.pop_front();

  if (onWriteQuiescence_ && writeBatcher_.empty() &&
      inflightWritesQueue_.empty()) {
    onWriteQuiescence_(ReadPausableHandle(this));
    return;
  }

  closeIfNeeded();
}

void RocketServerConnection::writeErr(
    size_t /* bytesWritten */, const folly::AsyncSocketException& ex) noexcept {
  DestructorGuard dg(this);
  DCHECK(!inflightWritesQueue_.empty());
  auto& context = inflightWritesQueue_.front(); // NB: Can be empty.
  for (auto processingCompleteCount = context.requestCompleteCount;
       processingCompleteCount > 0;
       --processingCompleteCount) {
    frameHandler_->requestComplete();
  }

  auto ew = folly::make_exception_wrapper<transport::TTransportException>(ex);
  for (auto& cb : context.sendCallbacks) {
    cb.release()->messageSendError(folly::copy(ew));
  }

  inflightWritesQueue_.pop_front();
  close(std::move(ew));
}

void RocketServerConnection::onEgressBuffered() {
  const auto buffered = rawSocket_->getAllocatedBytesBuffered();
  const auto oldBuffered = egressBufferSize_;
  egressBufferSize_ = buffered;
  // track egress memory consumption, drop connection if necessary
  if (buffered < oldBuffered) {
    const auto delta = oldBuffered - buffered;
    egressMemoryTracker_.decrement(delta);
    DVLOG(10) << fmt::format("buffered: {} (-{}) B", buffered, delta);
  } else {
    const auto delta = buffered - oldBuffered;
    const auto exceeds = !egressMemoryTracker_.increment(delta);
    DVLOG(10) << fmt::format("buffered: {} (+{}) B", buffered, delta);
    if (exceeds && rawSocket_->good()) {
      DestructorGuard dg(this);
      FB_LOG_EVERY_MS(ERROR, 1000) << fmt::format(
          "Dropping connection for ({}): exceeded egress memory limit ({}). The config value of min increment size: ({})",
          getPeerAddress().describe(),
          egressMemoryTracker_.getMemLimit(),
          egressMemoryTracker_.getMinIncrementSize());
      if (auto context = frameHandler_->getCpp2ConnContext()) {
        THRIFT_CONNECTION_EVENT(exceeded_egress_mem_limit).log(*context, [&] {
          folly::dynamic metadata = folly::dynamic::object;
          metadata["mem_limit"] = egressMemoryTracker_.getMemLimit();
          metadata["min_increment_size"] =
              egressMemoryTracker_.getMinIncrementSize();
          return metadata;
        });
      }

      rawSocket_->closeNow(); // triggers writeErr() events now
      return;
    }
  }
  // pause streams if buffer size reached backpressure threshold
  if (!egressBufferBackpressureThreshold_) {
    return;
  } else if (buffered > egressBufferBackpressureThreshold_ && !streamsPaused_) {
    pauseStreams();
  } else if (streamsPaused_ && buffered < egressBufferRecoverySize_) {
    resumeStreams();
  }
}

void RocketServerConnection::onEgressBufferCleared() {
  if (egressBufferSize_) {
    egressMemoryTracker_.decrement(egressBufferSize_);
    DVLOG(10) << "buffered: 0 (-" << egressBufferSize_ << ") B";
    egressBufferSize_ = 0;
  }
  if (UNLIKELY(streamsPaused_)) {
    resumeStreams();
  }
}

void RocketServerConnection::scheduleStreamTimeout(
    folly::HHWheelTimer::Callback* timeoutCallback) {
  if (streamStarvationTimeout_ != std::chrono::milliseconds::zero()) {
    evb_.timer().scheduleTimeout(timeoutCallback, streamStarvationTimeout_);
  }
}

void RocketServerConnection::scheduleSinkTimeout(
    folly::HHWheelTimer::Callback* timeoutCallback,
    std::chrono::milliseconds timeout) {
  if (timeout != std::chrono::milliseconds::zero()) {
    evb_.timer().scheduleTimeout(timeoutCallback, timeout);
  }
}

void RocketServerConnection::sendPayload(
    StreamId streamId,
    Payload&& payload,
    Flags flags,
    apache::thrift::MessageChannel::SendCallbackPtr cb) {
  auto fds = std::move(payload.fds);
  send(
      PayloadFrame(streamId, std::move(payload), flags).serialize(),
      std::move(cb),
      streamId,
      std::move(fds));
}

void RocketServerConnection::sendError(
    StreamId streamId,
    RocketException&& rex,
    apache::thrift::MessageChannel::SendCallbackPtr cb) {
  send(
      ErrorFrame(streamId, std::move(rex)).serialize(),
      std::move(cb),
      streamId);
}

void RocketServerConnection::sendRequestN(StreamId streamId, int32_t n) {
  send(RequestNFrame(streamId, n).serialize(), nullptr, streamId);
}

void RocketServerConnection::sendCancel(StreamId streamId) {
  send(CancelFrame(streamId).serialize(), nullptr, streamId);
}

void RocketServerConnection::sendMetadataPush(
    std::unique_ptr<folly::IOBuf> metadata) {
  send(MetadataPushFrame::makeFromMetadata(std::move(metadata)).serialize());
}

void RocketServerConnection::freeStream(
    StreamId streamId, bool markRequestComplete) {
  // If we crash here, there is a possible data race on accessing `streams_`.
  // The IOThread could be adding streams to the map concurrently, or erase
  // here could invalidate any iterator being held by the IOThread (F14FastMap
  // does not guarantee iterator stability on erase()).
  if (THRIFT_FLAG(thrift_check_free_stream_thread)) {
    getEventBase().checkIsInEventBaseThread();
  }

  DestructorGuard dg(this);

  if (auto it = streams_.find(streamId); it != streams_.end()) {
    streams_.erase(it);

    // We won't mark request complete if the stream was already erased
    if (markRequestComplete) {
      requestComplete();
    }
  }
}

void RocketServerConnection::applyQosMarking(
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

void RocketServerConnection::pauseStreams() {
  DCHECK(!streamsPaused_);
  streamsPaused_ = true;
  for (auto& [_, handler] : streams_) {
    handler->handlePausedByConnection();
  }
}

void RocketServerConnection::resumeStreams() {
  DCHECK(streamsPaused_);
  streamsPaused_ = false;
  for (auto& [_, handler] : streams_) {
    handler->handleResumedByConnection();
  }
}

bool RocketServerConnection::incMemoryUsage(uint32_t memSize) {
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

RocketServerHandler& RocketServerConnection::getFrameHandler() {
  return *frameHandler_;
}

std::vector<InteractionInfo> RocketServerConnection::getInteractionSnapshots()
    const {
  std::vector<InteractionInfo> result;
  if (auto* ctx = frameHandler_->getCpp2ConnContext()) {
    apache::thrift::detail::Cpp2ConnContextInternalAPI api(*ctx);
    api.forEachTile([&result](int64_t id, Tile& tile) {
      apache::thrift::detail::TileInternalAPI tileApi(tile);
      result.push_back(
          InteractionInfo{
              id,
              tile.getInteractionCreationTime(),
              tileApi.getLastActivityTime(),
              tileApi.getRefCount()});
    });
  }
  return result;
}

void RocketServerConnection::terminateInteraction(int64_t id) {
  if (evb_.isInEventBaseThread()) {
    frameHandler_->terminateInteraction(id);
  } else {
    evb_.runInEventBaseThread(
        [this, dg = folly::DelayedDestruction::DestructorGuard(this), id] {
          frameHandler_->terminateInteraction(id);
        });
  }
}

} // namespace apache::thrift::rocket
