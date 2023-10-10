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

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/PayloadUtils.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionPlugins.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>

THRIFT_FLAG_DEFINE_bool(enable_rocket_connection_observers, false);

namespace apache {
namespace thrift {
namespace rocket {

constexpr std::chrono::seconds RocketServerConnection::SocketDrainer::kTimeout;

RocketServerConnection::RocketServerConnection(
    folly::AsyncTransport::UniquePtr socket,
    std::unique_ptr<RocketServerHandler> frameHandler,
    MemoryTracker& ingressMemoryTracker,
    MemoryTracker& egressMemoryTracker,
    const Config& cfg)
    : evb_(*socket->getEventBase()),
      socket_(std::move(socket)),
      rawSocket_(
          socket_ ? socket_->getUnderlyingTransport<folly::AsyncSocket>()
                  : nullptr),
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
      egressMemoryTracker_(egressMemoryTracker),
      observerContainer_(this) {
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
        if (auto err = option.apply(sockfd, value)) {
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
}

RocketStreamClientCallback* FOLLY_NULLABLE
RocketServerConnection::createStreamClientCallback(
    StreamId streamId,
    RocketServerConnection& connection,
    uint32_t initialRequestN) {
  auto [it, inserted] = streams_.try_emplace(streamId);
  if (!inserted) {
    return nullptr;
  }
  auto cb = std::make_unique<RocketStreamClientCallback>(
      streamId, connection, initialRequestN);
  auto cbPtr = cb.get();
  it->second = std::move(cb);
  return cbPtr;
}

RocketSinkClientCallback* FOLLY_NULLABLE
RocketServerConnection::createSinkClientCallback(
    StreamId streamId, RocketServerConnection& connection) {
  auto [it, inserted] = streams_.try_emplace(streamId);
  if (!inserted) {
    return nullptr;
  }
  auto cb = std::make_unique<RocketSinkClientCallback>(streamId, connection);
  auto cbPtr = cb.get();
  it->second = std::move(cb);
  return cbPtr;
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

    // Our writeSuccess / writeError handlers require that every write to
    // the socket be preceded by a matched queue entry.  To avoid complex
    // unbatching of `WriteBatchContext`s in `WriteBatcher::enqueueWrite`,
    // let's make the first N-1 queue entries empty dummies, and use the
    // full batched context for the last write.
    inflightWritesQueue_.push_back(
        writesQ.empty() ? std::move(context) : WriteBatchContext{});
    // KEEP THIS INVARIANT: For the receiver to correctly match FDs to a
    // message, the FDs must be sent with the IOBuf ending with the FINAL
    // fragment of that message.  Today, message fragments are not
    // interleaved, so there is no explicit logic around this, but this
    // invariant must be preserved going forward.
    writeChainWithFds(socket_.get(), this, std::move(write), std::move(fds));

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
  evb_.dcheckIsInEventBaseThread();

  if (state_ != ConnectionState::ALIVE && state_ != ConnectionState::DRAINING) {
    return;
  }

  writeBatcher_.enqueueWrite(
      std::move(data), std::move(cb), streamId, std::move(fds));
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
      serverMeta.drainCompletePush_ref()
          .ensure()
          .drainCompleteCode_ref()
          .from_optional(drainCompleteCode_);
      sendMetadataPush(packCompact(std::move(serverMeta)));
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
        [](const std::unique_ptr<RocketStreamClientCallback>& callback) {
          callback->onStreamCancel();
        },
        [](const std::unique_ptr<RocketSinkClientCallback>& callback) {
          bool state = callback->onSinkError(TApplicationException(
              TApplicationException::TApplicationExceptionType::INTERRUPTION));
          DCHECK(state) << "onSinkError called after sink complete!";
        });
    requestComplete();
  }

  writeBatcher_.drain();

  destroy();
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
      return close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID_SETUP, "First frame must be SETUP frame"));
    }
    setupFrameReceived_ = true;
  } else {
    if (UNLIKELY(frameType == FrameType::SETUP)) {
      return close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID_SETUP, "More than one SETUP frame received"));
    }
  }

  switch (frameType) {
    case FrameType::SETUP: {
      return frameHandler_->handleSetupFrame(
          SetupFrame(std::move(frame)), *this);
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
          send(KeepAliveFrame{Flags(), std::move(keepAliveFrame).data()}
                   .serialize());
        }
      } else {
        close(folly::make_exception_wrapper<RocketException>(
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
        folly::variant_match(
            iter->second,
            [&](const std::unique_ptr<RocketStreamClientCallback>&
                    clientCallback) {
              handleStreamFrame(
                  std::move(frame),
                  streamId,
                  frameType,
                  flags,
                  std::move(cursor),
                  *clientCallback);
            },
            [&](const std::unique_ptr<RocketSinkClientCallback>&
                    clientCallback) {
              handleSinkFrame(
                  std::move(frame),
                  streamId,
                  frameType,
                  flags,
                  std::move(cursor),
                  *clientCallback);
            });
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
      FOLLY_FALLTHROUGH;
    case FrameType::REQUEST_N:
      FOLLY_FALLTHROUGH;
    case FrameType::ERROR:
      return;

    case FrameType::EXT: {
      ExtFrame extFrame(streamId, flags, cursor, std::move(frame));
      if (!extFrame.hasIgnore()) {
        close(folly::make_exception_wrapper<RocketException>(
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
        unpackCompact(clientMeta, metadataFrame.metadata());
      } catch (...) {
        close(folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID, "Failed to deserialize metadata push frame"));
        return;
      }
      switch (clientMeta.getType()) {
        case ClientPushMetadata::Type::interactionTerminate: {
          frameHandler_->terminateInteraction(
              *clientMeta.interactionTerminate_ref()->interactionId_ref());
          break;
        }
        case ClientPushMetadata::Type::streamHeadersPush: {
          StreamId sid(
              clientMeta.streamHeadersPush_ref()->streamId_ref().value_or(0));
          auto it = streams_.find(sid);
          if (it != streams_.end()) {
            folly::variant_match(
                it->second,
                [&](const std::unique_ptr<RocketStreamClientCallback>&
                        clientCallback) {
                  std::ignore =
                      clientCallback->getStreamServerCallback().onSinkHeaders(
                          HeadersPayload(clientMeta.streamHeadersPush_ref()
                                             ->headersPayloadContent_ref()
                                             .value_or({})));
                },
                [&](const std::unique_ptr<RocketSinkClientCallback>&) {});
          }
          break;
        }
        case ClientPushMetadata::Type::transportMetadataPush: {
          if (auto context = frameHandler_->getCpp2ConnContext()) {
            auto md =
                clientMeta.transportMetadataPush_ref()->transportMetadata_ref();
            THRIFT_CONNECTION_EVENT(transport.metadata).log(*context, [&] {
              folly::dynamic transportMetadata = folly::dynamic::object;
              if (md) {
                for (auto p : *md) {
                  transportMetadata[p.first] = p.second;
                }
              }
              return transportMetadata;
            });
          }
          break;
        }
        default:
          break;
      }
      return;
    }

    default:
      close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID,
          fmt::format(
              "Received unhandleable frame type ({})",
              static_cast<uint8_t>(frameType))));
  }
}

void RocketServerConnection::handleStreamFrame(
    std::unique_ptr<folly::IOBuf> frame,
    StreamId streamId,
    FrameType frameType,
    Flags flags,
    folly::io::Cursor cursor,
    RocketStreamClientCallback& clientCallback) {
  if (!clientCallback.serverCallbackReady()) {
    switch (frameType) {
      case FrameType::CANCEL: {
        return clientCallback.earlyCancelled();
      }
      default:
        return close(folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unexpected early frame, stream id ({}) type ({})",
                static_cast<uint32_t>(streamId),
                static_cast<uint8_t>(frameType))));
    }
  }

  switch (frameType) {
    case FrameType::REQUEST_N: {
      RequestNFrame requestNFrame(streamId, flags, cursor);
      clientCallback.request(requestNFrame.requestN());
      return;
    }

    case FrameType::CANCEL: {
      clientCallback.onStreamCancel();
      freeStream(streamId, true);
      return;
    }

    case FrameType::EXT: {
      ExtFrame extFrame(streamId, flags, cursor, std::move(frame));
      if (!extFrame.hasIgnore()) {
        close(folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "Received unhandleable EXT frame type ({}) for stream (id {})",
                static_cast<uint32_t>(extFrame.extFrameType()),
                static_cast<uint32_t>(streamId))));
      }
      return;
    }

    default:
      close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID,
          fmt::format(
              "Received unhandleable frame type ({}) for stream (id {})",
              static_cast<uint8_t>(frameType),
              static_cast<uint32_t>(streamId))));
  }
}

void RocketServerConnection::handleSinkFrame(
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
    return close(folly::make_exception_wrapper<RocketException>(
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
        auto streamPayload =
            rocket::unpack<StreamPayload>(std::move(*fullPayload));
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

          auto payloadMetadataRef =
              streamPayload->metadata.payloadMetadata_ref();
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
        // it is possible final repsonse(error) sent from serverCallback,
        // serverCallback may be already destoryed.
        if (streams_.find(streamId) != streams_.end()) {
          notViolateContract = clientCallback.onSinkComplete();
        }
      }

      if (!notViolateContract) {
        close(folly::make_exception_wrapper<transport::TTransportException>(
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
        close(folly::make_exception_wrapper<transport::TTransportException>(
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
      close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID,
          fmt::format(
              "Received unhandleable frame type ({}) for sink (id {})",
              static_cast<uint8_t>(frameType),
              static_cast<uint32_t>(streamId))));
  }
}

void RocketServerConnection::close(folly::exception_wrapper ew) {
  if (state_ == ConnectionState::CLOSING || state_ == ConnectionState::CLOSED) {
    closeIfNeeded();
    return;
  }

  DestructorGuard dg(this);

  socketDrainer_.activate();

  if (!ew.with_exception<RocketException>([this](RocketException rex) {
        sendError(StreamId{0}, std::move(rex));
      })) {
    auto rex = ew
        ? RocketException(ErrorCode::CONNECTION_ERROR, ew.what())
        : RocketException(ErrorCode::CONNECTION_CLOSE, "Closing connection");
    sendError(StreamId{0}, std::move(rex));
  }

  state_ = ConnectionState::CLOSING;
  frameHandler_->connectionClosing();
  closeIfNeeded();
}

void RocketServerConnection::timeoutExpired() noexcept {
  DestructorGuard dg(this);

  if (!isBusy()) {
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

  close(folly::make_exception_wrapper<transport::TTransportException>(
      transport::TTransportException::TTransportExceptionType::INTERRUPTED,
      "Dropping connection"));
}

void RocketServerConnection::closeWhenIdle() {
  socketDrainer_.drainComplete();
  close(folly::make_exception_wrapper<transport::TTransportException>(
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
          "Dropping connection: exceeded egress memory limit ({})",
          getPeerAddress().describe());
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

folly::Optional<Payload> RocketServerConnection::bufferOrGetFullPayload(
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
  DestructorGuard dg(this);

  bufferedFragments_.erase(streamId);

  DCHECK(streams_.find(streamId) != streams_.end());
  streams_.erase(streamId);
  if (markRequestComplete) {
    requestComplete();
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
        << folly::exceptionStr(std::current_exception());
  }
}

RocketServerConnection::ReadResumableHandle::ReadResumableHandle(
    RocketServerConnection* connection)
    : connection_(connection) {}

RocketServerConnection::ReadResumableHandle::~ReadResumableHandle() {
  if (connection_ != nullptr) {
    std::move(*this).resume();
  }
}

RocketServerConnection::ReadResumableHandle::ReadResumableHandle(
    ReadResumableHandle&& handle) noexcept
    : connection_(std::exchange(handle.connection_, nullptr)) {}

RocketServerConnection::ReadPausableHandle::ReadPausableHandle(
    RocketServerConnection* connection)
    : connection_(connection) {
  ++connection_->activePausedHandlers_;
}

void RocketServerConnection::ReadResumableHandle::resume() && {
  DCHECK(connection_ != nullptr) << "resume() has been called on this handle";
  --connection_->activePausedHandlers_;
  if (connection_->state_ == ConnectionState::ALIVE ||
      connection_->state_ == ConnectionState::DRAINING) {
    connection_->socket_->setReadCB(&connection_->parser_);
  }
  connection_->closeIfNeeded();
  connection_ = nullptr;
}

RocketServerConnection::ReadPausableHandle::~ReadPausableHandle() {
  if (connection_ != nullptr) {
    --connection_->activePausedHandlers_;
    connection_->closeIfNeeded();
  }
}

RocketServerConnection::ReadPausableHandle::ReadPausableHandle(
    ReadPausableHandle&& handle) noexcept
    : connection_(std::exchange(handle.connection_, nullptr)) {}

RocketServerConnection::ReadResumableHandle
RocketServerConnection::ReadPausableHandle::pause() && {
  DCHECK(connection_ != nullptr) << "pause() has been called on this handle";
  connection_->socket_->setReadCB(nullptr);
  return ReadResumableHandle(std::exchange(connection_, nullptr));
}

void RocketServerConnection::pauseStreams() {
  DCHECK(!streamsPaused_);
  streamsPaused_ = true;
  for (auto it = streams_.begin(); it != streams_.end(); it++) {
    folly::variant_match(
        it->second,
        [](const std::unique_ptr<RocketStreamClientCallback>& stream) {
          stream->pauseStream();
        },
        [](const auto&) {});
  }
}

void RocketServerConnection::resumeStreams() {
  DCHECK(streamsPaused_);
  streamsPaused_ = false;
  for (auto it = streams_.begin(); it != streams_.end(); it++) {
    folly::variant_match(
        it->second,
        [](const std::unique_ptr<RocketStreamClientCallback>& stream) {
          stream->resumeStream();
        },
        [](const auto&) {});
  }
}

} // namespace rocket
} // namespace thrift
} // namespace apache
