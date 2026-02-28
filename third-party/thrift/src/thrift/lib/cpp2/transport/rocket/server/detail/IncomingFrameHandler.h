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

#include <fmt/core.h>
#include <folly/Overload.h>
#include <folly/String.h>
#include <folly/io/Cursor.h>
#include <folly/lang/Bits.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/core/FrameUtil.h>
#include <thrift/lib/cpp2/transport/rocket/core/StreamUtil.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/ConnectionAdapter.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/ExistingStreamFrameHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/KeepAliveHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/MetadataPushHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RequestChannelHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RequestFnfHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RequestResponseHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RequestStreamHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/SetupFrameAcceptor.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/StreamCallbackManager.h>

namespace apache::thrift::rocket {

[[maybe_unused]] FOLLY_ALWAYS_INLINE FrameType
parseFrameType(folly::IOBuf& frame) noexcept {
  constexpr size_t kFlagAndFrameSize = sizeof(uint16_t);
  constexpr size_t kStreamIdSize = sizeof(uint32_t);
  FOLLY_SAFE_DCHECK(
      frame.computeChainDataLength() >= kFlagAndFrameSize + kStreamIdSize,
      "frame to small for RSocket Stream Id and Flag - must be at least as long 40-bits long");

  // Use cursor to safely read across IOBuf chain boundaries
  folly::io::Cursor cursor(&frame);

  // Skip the stream ID (4 bytes)
  cursor.skip(kStreamIdSize);

  // Read the frame type and flags (2 bytes) in big-endian format
  uint16_t rawFrameType = cursor.readBE<uint16_t>();
  return static_cast<FrameType>(rawFrameType >> Flags::kBits);
}

/**
 * Handles incoming frames. Deserializes the frames and determines how to
 * delegate the handling of the frame. This is to decouple the parsing of the
 * frames from the handling of the frames. This class is per-connection.
 */
template <
    typename ConnectionT,
    template <typename> class ConnectionAdapter,
    typename RocketServerHandler,
    typename RequestStreamCallback,
    typename RequestChannelCallback,
    typename RocketServerFrameContext>
class IncomingFrameHandler {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  IncomingFrameHandler(
      Connection& connection,
      SetupFrameAcceptor<ConnectionT, ConnectionAdapter, RocketServerHandler>&
          setupFrameAcceptor,
      RequestResponseHandler<ConnectionT, ConnectionAdapter>&
          requestResponseHandler,
      RequestFnfHandler<
          ConnectionT,
          ConnectionAdapter,
          RocketServerFrameContext>& requestFnfHandler,
      KeepAliveHandler<ConnectionT, ConnectionAdapter>& keepAliveFrameHandler,
      MetadataPushHandler<ConnectionT, ConnectionAdapter>& metadataPushHandler,
      StreamCallbackManager<
          ConnectionT,
          ConnectionAdapter,
          RocketStreamClientCallback>& streamCallbackManager,
      RequestStreamHandler<
          ConnectionT,
          ConnectionAdapter,
          RocketServerFrameContext>& requestStreamHandler,
      RequestChannelHandler<
          ConnectionT,
          ConnectionAdapter,
          RocketServerFrameContext>& requestChannelHandler,
      ExistingStreamFrameHandler<
          ConnectionT,
          ConnectionAdapter,
          RequestStreamCallback,
          RequestChannelCallback,
          RocketServerFrameContext>& existingStreamFrameHandler)
      : connection_(&connection),
        setupFrameAcceptor_(&setupFrameAcceptor),
        requestResponseHandler_(&requestResponseHandler),
        requestFnfHandler_(&requestFnfHandler),
        keepAliveHandler_(&keepAliveFrameHandler),
        metadataPushHandler_(&metadataPushHandler),
        streamCallbackManager_(&streamCallbackManager),
        requestStreamHandler_(&requestStreamHandler),
        requestChannelHandler_(&requestChannelHandler),
        existingStreamFrameHandler_(&existingStreamFrameHandler) {}

  void handle(std::unique_ptr<folly::IOBuf>&& frame) {
    // hexDumpFrame(*frame);
    auto frameType = parseFrameType(*frame);
    switch (frameType) {
      case FrameType::SETUP:
        handleSetupFrame(std::move(frame));
        break;
      case FrameType::REQUEST_RESPONSE:
        handleFrame<FrameType::REQUEST_RESPONSE>(
            std::move(frame), requestResponseHandler_);
        break;
      case FrameType::REQUEST_STREAM:
        handleFrame<FrameType::REQUEST_STREAM>(
            std::move(frame), requestStreamHandler_);
        break;
      case FrameType::REQUEST_N:
        handleFrame<FrameType::REQUEST_N>(
            std::move(frame), existingStreamFrameHandler_);
        break;
      case FrameType::CANCEL:
        handleFrame<FrameType::CANCEL>(
            std::move(frame), existingStreamFrameHandler_);
        break;
      case FrameType::REQUEST_FNF:
        handleFrame<FrameType::REQUEST_FNF>(
            std::move(frame), requestFnfHandler_);
        break;
      case FrameType::REQUEST_CHANNEL:
        handleFrame<FrameType::REQUEST_CHANNEL>(
            std::move(frame), requestChannelHandler_);
        break;
      case FrameType::PAYLOAD:
        handleFrame<FrameType::PAYLOAD>(
            std::move(frame), existingStreamFrameHandler_);
        break;
      // TODO Add Support for ERROR frame type
      // case FrameType::ERROR:
      //   handleFrame<FrameType::ERROR>(std::move(frame),
      //   existingStreamFrameHandler_); break;
      case FrameType::METADATA_PUSH:
        handleFrame<FrameType::METADATA_PUSH>(
            std::move(frame), metadataPushHandler_);
        break;
      case FrameType::KEEPALIVE:
        handleFrame<FrameType::KEEPALIVE>(std::move(frame), keepAliveHandler_);
        break;
      case FrameType::EXT:
        handleFrame<FrameType::EXT>(
            std::move(frame), existingStreamFrameHandler_);
        break;
      default:
        hexDumpFrame(*frame);
        handleUnknownFrame(frameType);
        break;
    }
  }

 private:
  bool receivedSetupFrame_{false};
  Connection* connection_;
  SetupFrameAcceptor<ConnectionT, ConnectionAdapter, RocketServerHandler>*
      setupFrameAcceptor_;
  RequestResponseHandler<ConnectionT, ConnectionAdapter>*
      requestResponseHandler_;
  RequestFnfHandler<ConnectionT, ConnectionAdapter, RocketServerFrameContext>*
      requestFnfHandler_;
  KeepAliveHandler<ConnectionT, ConnectionAdapter>* keepAliveHandler_;
  MetadataPushHandler<ConnectionT, ConnectionAdapter>* metadataPushHandler_;
  StreamCallbackManager<
      ConnectionT,
      ConnectionAdapter,
      RocketStreamClientCallback>* streamCallbackManager_;
  RequestStreamHandler<
      ConnectionT,
      ConnectionAdapter,
      RocketServerFrameContext>* requestStreamHandler_;
  RequestChannelHandler<
      ConnectionT,
      ConnectionAdapter,
      RocketServerFrameContext>* requestChannelHandler_;
  ExistingStreamFrameHandler<
      ConnectionT,
      ConnectionAdapter,
      RequestStreamCallback,
      RequestChannelCallback,
      RocketServerFrameContext>* existingStreamFrameHandler_;

  FOLLY_ALWAYS_INLINE void hexDumpFrame(folly::IOBuf& frame) const noexcept {
    // if constexpr (folly::kIsDebug) {
    XLOG(INFO)
        << "Handling Frame from: "
        << connection_->getWrappedConnection()->getPeerAddress().describe()
        << std::endl
        << folly::hexDump(
               frame.coalesce().data(), frame.computeChainDataLength());
    //}
  }

  void handleSetupFrame(std::unique_ptr<folly::IOBuf> frame) noexcept {
    if (std::exchange(receivedSetupFrame_, true)) {
      XLOG(WARN) << "More than one SETUP frame received";
      connection_->close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID_SETUP, "More than one SETUP frame received"));
    } else {
      auto setupFrame =
          frame_util::FrameTypeToTraits_t<FrameType::SETUP>::deserialize(
              std::move(frame));
      setupFrameAcceptor_->handle(std::move(setupFrame));
    }
  }

  void handleUnknownFrame(FrameType frameType) {
    connection_->close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID,
            fmt::format(
                "$$$ Received unhandleable frame type ({})",
                static_cast<uint8_t>(frameType))));
  }

  bool hasValidSetupFrameState() const noexcept {
    bool valid = true;
    if (FOLLY_UNLIKELY(receivedSetupFrame_ == false)) {
      connection_->close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID_SETUP, "First frame must be SETUP frame"));
      valid = false;
    }
    return valid;
  }

  folly::exception_wrapper invalidStreamingStreamIdException(
      FrameType frameType, uint32_t streamId) {
    return folly::make_exception_wrapper<RocketException>(
        ErrorCode::INVALID,
        fmt::format(
            "Received unhandleable frame type ({}) for stream (id {})",
            static_cast<uint8_t>(frameType),
            streamId));
  }

  folly::exception_wrapper invalidMetadataSreamIdException(uint32_t streamId) {
    return folly::make_exception_wrapper<RocketException>(
        ErrorCode::CONNECTION_ERROR,
        fmt::format(
            "Received metadatapush frame with non-zero stream ID {}",
            streamId));
  }

  folly::exception_wrapper invalidKeepAliveStreamIdException(
      uint32_t streamId) {
    return folly::make_exception_wrapper<RocketException>(
        ErrorCode::CONNECTION_ERROR,
        fmt::format(
            "Received keepalive frame with non-zero stream ID {}", streamId));
  }

  template <FrameType type>
  FOLLY_ALWAYS_INLINE folly::exception_wrapper invalidStreamIdException(
      uint32_t streamId) {
    if constexpr (stream_util::isKeepAliveFrameType<type>()) {
      return invalidKeepAliveStreamIdException(streamId);
    } else if (stream_util::isMetadataPushFrameType<type>()) {
      return invalidMetadataSreamIdException(streamId);
    } else {
      return invalidStreamingStreamIdException(type, streamId);
    }
  }

  template <FrameType type, typename Handler>
  void handleFrame(
      std::unique_ptr<folly::IOBuf> buffer, Handler* handler) noexcept {
    using Traits = frame_util::FrameTypeToTraits_t<type>;

    if (FOLLY_UNLIKELY(receivedSetupFrame_ == false)) {
      XLOG(DBG4) << "Received frame before setup frame for raw frame type "
                 << frame_util::rawFrameType<type>() << std::endl;
      connection_->close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID_SETUP, "First frame must be SETUP frame"));
      return;
    }

    auto frame = Traits::deserialize(std::move(buffer));

    auto isInvalidStreamId = Traits::isStreamZeroFrame()
        ? stream_util::isStreamIdNonZero(frame)
        : stream_util::isStreamIdZero(frame);
    if (FOLLY_UNLIKELY(isInvalidStreamId)) {
      XLOG(DBG4) << "Received frame on invalid stream id for raw frame type "
                 << frame_util::rawFrameType<type>() << " and raw stream id "
                 << stream_util::rawStreamId(frame) << std::endl;
      connection_->close(
          invalidStreamIdException<type>(stream_util::rawStreamId(frame)));
      return;
    }

    handler->handle(std::move(frame));
  }
};

} // namespace apache::thrift::rocket
