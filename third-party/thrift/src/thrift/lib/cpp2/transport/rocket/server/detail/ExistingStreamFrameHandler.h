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
#include <folly/Likely.h>
#include <folly/Overload.h>
#include <folly/io/Cursor.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/core/FrameUtil.h>
#include <thrift/lib/cpp2/transport/rocket/core/StreamUtil.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket {

class RocketServerFrameContext;

/**
 * Handles frames for EXISTING streams (REQUEST_N, CANCEL, EXT, and future
 * PAYLOAD/CHANNEL frames).
 */
template <
    typename ConnectionT,
    template <typename> class ConnectionAdapter,
    typename RequestStreamCallback,
    typename RequestChannelCallback,
    typename RocketServerFrameContext>
class ExistingStreamFrameHandler {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit ExistingStreamFrameHandler(Connection* connection) noexcept
      : connection_(connection) {}

  /**
   * Handle any frame for an existing stream. Routes to appropriate frame
   * handling function. Will fail to compile if passed a frame type that is not
   * handled.
   */
  template <typename Frame>
  void handle(Frame&& frame) noexcept {
    // Extract stream ID for stream lookup
    const StreamId streamId = frame.streamId();
    auto it = connection_->getWrappedConnection()->findStream(streamId);

    if (it == connection_->getWrappedConnection()->streamsEnd()) {
      // Stream not found - this is an error condition
      connection_->close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID,
              fmt::format(
                  "Received frame for non-existent stream ID: {}",
                  static_cast<uint32_t>(streamId))));
      return;
    }

    if constexpr (std::is_same_v<std::decay_t<Frame>, PayloadFrame>) {
      handlePayloadFrame(std::forward<Frame>(frame));
    } else if constexpr (std::is_same_v<std::decay_t<Frame>, RequestNFrame>) {
      handleRequestNFrame(std::forward<Frame>(frame), it);
    } else if constexpr (std::is_same_v<std::decay_t<Frame>, CancelFrame>) {
      handleCancelFrame(std::forward<Frame>(frame), it);
    } else if constexpr (std::is_same_v<std::decay_t<Frame>, ExtFrame>) {
      handleExtFrame(std::forward<Frame>(frame));
    } else {
      static_assert(
          std::is_same_v<Frame, Frame>,
          "Frame type not handled by ExistingStreamFrameHandler");
    }
  }

 private:
  Connection* connection_;

  void handlePayloadFrame(PayloadFrame&& payloadFrame) noexcept {
    auto& partialRequestFrames = connection_->getPartialRequestFrames();
    const StreamId streamId = payloadFrame.streamId();
    auto it = partialRequestFrames.find(streamId);

    if (it != partialRequestFrames.end()) {
      // Handle partial request frames (hasFollows reassembly)
      folly::variant_match(it->second, [&](auto& requestFrame) {
        const bool hasFollows = payloadFrame.hasFollows();
        requestFrame.payload().append(std::move(payloadFrame.payload()));
        if (!hasFollows) {
          RocketServerFrameContext(
              *connection_->getWrappedConnection(), streamId)
              .onFullFrame(std::move(requestFrame));
          partialRequestFrames.erase(streamId);
        }
      });
    } else {
      // Handle sink payload frames - these go to existing sink callbacks
      handleSinkPayloadFrame(streamId, std::move(payloadFrame));
    }
  }

  /**
   * Handle REQUEST_N frames - routes to stream or sink based on callback type.
   * Uses delegation to existing callbacks to avoid circular dependencies.
   */
  template <typename IteratorType>
  void handleRequestNFrame(RequestNFrame&& frame, IteratorType& it) noexcept {
    folly::variant_match(
        it->second,
        [&](std::unique_ptr<RequestStreamCallback>& clientCallback) {
          if (clientCallback) {
            clientCallback->request(frame.requestN());
          }
        },
        [&](std::unique_ptr<RequestChannelCallback>& clientCallback) {
          if (clientCallback) {
            clientCallback->onSinkRequestN(frame.requestN());
          }
        },
        [&](auto& clientCallback) {
          // Handle other variants like BiDi callbacks
          // For now, skip handling BiDi callbacks - they will be handled
          // specifically later
          (void)clientCallback;
        });
  }

  /**
   * Handle CANCEL frames - interface matches handleFrame expectation.
   * Uses pure delegation to avoid circular dependencies.
   */
  template <typename IteratorType>
  void handleCancelFrame(CancelFrame&&, IteratorType& it) noexcept {
    folly::variant_match(
        it->second,
        [&](std::unique_ptr<RequestStreamCallback>& clientCallback) {
          if (clientCallback) {
            clientCallback->onStreamCancel();
          }
        },
        [&](std::unique_ptr<RequestChannelCallback>& clientCallback) {
          if (clientCallback) {
            // Note: we can receive connection closure AFTER we already
            // cancelled the sink
            clientCallback->onSinkError(TApplicationException(
                TApplicationException::TApplicationExceptionType::
                    INTERRUPTION));
          }
        },
        [&](auto& clientCallback) {
          // Handle other variants like BiDi callbacks
          if (clientCallback) {
            clientCallback->onStreamCancel();
          }
        });
  }

  /**
   * Handle EXT frames - interface matches handleFrame expectation.
   * Uses pure delegation to avoid circular dependencies.
   */
  void handleExtFrame(ExtFrame&&) noexcept {
    XLOG(FATAL) << "ext not implemented yet";
  }

  void handleSinkPayloadFrame(StreamId, PayloadFrame&&) noexcept {
    XLOG(FATAL) << "sink payload not implemented yet";
  }
};

} // namespace apache::thrift::rocket
