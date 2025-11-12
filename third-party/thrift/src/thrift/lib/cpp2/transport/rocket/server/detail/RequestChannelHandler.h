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

#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket {

/**
 * Handles REQUEST_CHANNEL frames for creating new bidirectional channels.
 * This handler is specific to sink/channel (REQUEST_CHANNEL) functionality
 * only.
 *
 * SCOPE LIMITATION: This class handles ONLY REQUEST_CHANNEL frames,
 * not REQUEST_STREAM or any other request types.
 */
template <
    typename ConnectionT,
    template <typename> class ConnectionAdapter,
    typename RocketServerFrameContext>
class RequestChannelHandler {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit RequestChannelHandler(Connection* connection) noexcept
      : connection_(connection) {}

  /**
   * Handles REQUEST_CHANNEL frames for creating new bidirectional channels.
   * Establishes sink semantics for client-to-server payload flow.
   * Extracts logic from RocketServerConnection for REQUEST_CHANNEL handling.
   *
   * @param frame The REQUEST_CHANNEL frame to process
   */
  void handle(RequestChannelFrame&& frame) noexcept {
    auto streamId = frame.streamId();
    if (FOLLY_UNLIKELY(frame.hasFollows())) {
      // Handle partial REQUEST_CHANNEL frames (hasFollows)
      connection_->emplacePartialRequestChannelFrame(
          streamId, std::move(frame));
    } else {
      // Complete REQUEST_CHANNEL frame - process immediately
      RocketServerFrameContext(*connection_->getWrappedConnection(), streamId)
          .onFullFrame(std::move(frame));
    }
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
