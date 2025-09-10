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

#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>

#include <utility>

#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>

namespace apache::thrift::rocket {

RocketServerFrameContext::RocketServerFrameContext(
    RocketServerConnection& connection, StreamId streamId)
    : connection_(&connection), streamId_(streamId) {
  connection_->incInflightRequests();
}

RocketServerFrameContext::RocketServerFrameContext(
    RocketServerFrameContext&& other) noexcept
    : connection_(other.connection_),
      streamId_(other.streamId_),
      markRequestComplete_(other.markRequestComplete_) {
  other.connection_ = nullptr;
}

RocketServerFrameContext::~RocketServerFrameContext() {
  if (connection_) {
    if (markRequestComplete_) {
      connection_->requestComplete();
    }
    connection_->decInflightRequests();
  }
}

folly::EventBase& RocketServerFrameContext::getEventBase() const {
  DCHECK(connection_);
  return connection_->getEventBase();
}

void RocketServerFrameContext::sendPayload(
    Payload&& payload,
    Flags flags,
    apache::thrift::MessageChannel::SendCallbackPtr cb) {
  DCHECK(connection_);
  DCHECK(flags.next() || flags.complete());
  connection_->sendPayload(streamId_, std::move(payload), flags, std::move(cb));
}

void RocketServerFrameContext::sendError(
    RocketException&& rex, apache::thrift::MessageChannel::SendCallbackPtr cb) {
  DCHECK(connection_);
  connection_->sendError(streamId_, std::move(rex), std::move(cb));
}

void RocketServerFrameContext::onFullFrame(
    RequestResponseFrame&& fullFrame) && {
  auto& frameHandler = *connection_->frameHandler_;
  frameHandler.handleRequestResponseFrame(
      std::move(fullFrame), std::move(*this));
}

void RocketServerFrameContext::onFullFrame(RequestFnfFrame&& fullFrame) && {
  auto& frameHandler = *connection_->frameHandler_;
  frameHandler.handleRequestFnfFrame(std::move(fullFrame), std::move(*this));
}

void RocketServerFrameContext::onFullFrame(RequestStreamFrame&& fullFrame) && {
  auto& connection = *connection_;
  auto& frameHandler = *connection.frameHandler_;
  if (auto clientCallback = connection.createStreamClientCallback(
          streamId_, connection, fullFrame.initialRequestN())) {
    frameHandler.handleRequestStreamFrame(
        std::move(fullFrame), std::move(*this), clientCallback);
  } else {
    connection.close(folly::make_exception_wrapper<
                     transport::TTransportException>(
        transport::TTransportException::TTransportExceptionType::
            STREAMING_CONTRACT_VIOLATION,
        fmt::format(
            "Received stream request frame with already in use stream id {}",
            static_cast<uint32_t>(streamId_))));
  }
}

void RocketServerFrameContext::onFullFrame(RequestChannelFrame&& fullFrame) && {
  auto& connection = *connection_;
  if (fullFrame.initialRequestN() < 2) {
    connection.close(
        folly::make_exception_wrapper<transport::TTransportException>(
            transport::TTransportException::TTransportExceptionType::
                STREAMING_CONTRACT_VIOLATION,
            "initialRequestN of Sink must be 2 or greater"));
  } else if (
      auto clientCallback = connection.createChannelClientCallback(
          streamId_, connection, fullFrame.initialRequestN())) {
    auto& frameHandler = *connection.frameHandler_;
    frameHandler.handleRequestChannelFrame(
        std::move(fullFrame), std::move(*this), *clientCallback);
  } else {
    connection.close(
        folly::make_exception_wrapper<transport::TTransportException>(
            transport::TTransportException::TTransportExceptionType::
                STREAMING_CONTRACT_VIOLATION,
            fmt::format(
                "Received sink request frame with already in use stream id {}",
                static_cast<uint32_t>(streamId_))));
  }
}

} // namespace apache::thrift::rocket
