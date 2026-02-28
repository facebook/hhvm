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

#include <thrift/lib/cpp2/transport/rocket/core/FrameUtil.h>

namespace apache::thrift::rocket {

/**
 * Handler for REQUEST_FNF (Fire-and-Forget) frames.
 *
 * Fire-and-forget requests are the simplest rocket frame type:
 * - One-way communication (no response expected)
 * - Stateless processing (no callbacks or stream management)
 * - Immediate processing (frame processed and discarded)
 * - Simple error handling (errors logged, not sent back to client)
 *
 * This handler follows the same template pattern as other frame handlers
 * to avoid circular dependencies through ConnectionAdapter abstraction.
 */
template <
    typename ConnectionT,
    template <typename> class ConnectionAdapter,
    typename RocketServerFrameContext>
class RequestFnfHandler {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit RequestFnfHandler(Connection* connection) noexcept
      : connection_(connection) {}

  /**
   * Handle REQUEST_FNF frames using the same pattern as RequestResponseHandler.
   *
   * For fire-and-forget requests:
   * - If frame has follows (partial frame), buffer it for completion
   * - If frame is complete, process immediately via RocketServerFrameContext
   * - No response is expected or sent back to client
   * - No stream callbacks are created (unlike REQUEST_STREAM)
   */
  void handle(RequestFnfFrame&& frame) noexcept {
    auto streamId = frame.streamId();
    if (UNLIKELY(frame.hasFollows())) {
      // Buffer partial frame until all fragments received
      connection_->emplacePartialFrame(streamId, std::move(frame));
    } else {
      // Process complete frame immediately - fire-and-forget has no lifecycle
      RocketServerFrameContext(*connection_->getWrappedConnection(), streamId)
          .onFullFrame(std::move(frame));
    }
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
