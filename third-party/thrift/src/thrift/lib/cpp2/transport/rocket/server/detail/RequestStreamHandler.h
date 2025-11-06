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

#include <folly/Likely.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/transport/rocket/core/FrameUtil.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket {

/**
 * Handles REQUEST_STREAM frames for creating new stream requests.
 * This handler is specific to streaming (REQUEST_STREAM) functionality only.
 *
 * SCOPE LIMITATION: This class handles ONLY REQUEST_STREAM frames,
 * not REQUEST_CHANNEL or any other request types.
 */
template <
    typename ConnectionT,
    template <typename> class ConnectionAdapter,
    typename RocketServerFrameContext>
class RequestStreamHandler {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit RequestStreamHandler(Connection* connection) noexcept
      : connection_(connection) {}

  /**
   * Handles REQUEST_STREAM frames for creating new streaming requests.
   * Extracts logic from RocketServerConnection for REQUEST_STREAM handling.
   *
   * @param frame The REQUEST_STREAM frame to process
   */
  void handle(RequestStreamFrame&& frame) noexcept {
    auto streamId = frame.streamId();
    if (FOLLY_UNLIKELY(frame.hasFollows())) {
      // Handle partial REQUEST_STREAM frames (hasFollows)
      connection_->emplacePartialFrame(streamId, std::move(frame));
    } else {
      // Complete REQUEST_STREAM frame - process immediately
      RocketServerFrameContext(*connection_->getWrappedConnection(), streamId)
          .onFullFrame(std::move(frame));
    }
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
