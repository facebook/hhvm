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

#include <folly/Indestructible.h>
#include <folly/Overload.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

#include <thrift/lib/cpp2/transport/rocket/server/detail/ConnectionAdapter.h>

namespace apache::thrift::rocket {

/**
 * Manages the lifecycle and operations of stream callback instances.
 * This class handles STREAMING-ONLY functionality (REQUEST_STREAM).
 *
 * SCOPE LIMITATION: This class handles ONLY stream callbacks,
 * not RocketSinkClientCallback or any sink/channel functionality.
 */
template <
    typename ConnectionT,
    template <typename> class ConnectionAdapter,
    typename RocketStreamClientCallback>
class StreamCallbackManager {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit StreamCallbackManager(Connection* connection) noexcept
      : connection_(connection) {}

  /**
   * Creates a new stream callback for REQUEST_STREAM handling.
   * Returns the created callback if successful, nullptr if streamId is already
   * in use.
   */
  RocketStreamClientCallback* FOLLY_NULLABLE
  createStreamClientCallback(StreamId streamId, uint32_t initialRequestN) {
    auto& streams = connection_->getStreams();
    auto [it, inserted] = streams.try_emplace(streamId);
    if (!inserted) {
      return nullptr;
    }

    auto cb = std::make_unique<RocketStreamClientCallback>(
        streamId, *connection_->getWrappedConnection(), initialRequestN);

    auto cbPtr = cb.get();
    it->second = std::move(cb);
    return cbPtr;
  }

  /**
   * Frees a stream callback and performs cleanup.
   * ONLY handles stream callback cleanup.
   *
   * @param streamId The stream ID to free
   * @param markRequestComplete Whether to mark the request as complete
   */
  void freeStream(StreamId streamId, bool markRequestComplete) {
    auto dg = connection_->getDestructorGuard();

    connection_->getBufferedFragments().erase(streamId);

    auto& streams = connection_->getStreams();
    DCHECK(streams.find(streamId) != streams.end());
    streams.erase(streamId);

    if (markRequestComplete) {
      connection_->requestComplete();
    }
  }

  /**
   * Pauses all active streams.
   * Called during backpressure situations.
   */
  void pauseStreams() {
    DCHECK(!connection_->areStreamsPaused());
    connection_->setStreamsPaused(true);

    auto& streams = connection_->getStreams();
    for (auto it = streams.begin(); it != streams.end(); it++) {
      folly::variant_match(
          it->second,
          [](const std::unique_ptr<RocketStreamClientCallback>& stream) {
            stream->handlePausedByConnection();
          },
          [](const auto&) {
            // Only handle stream callbacks - ignore sinks/channels
          });
    }
  }

  /**
   * Resumes all paused streams.
   * Called when backpressure is relieved.
   */
  void resumeStreams() {
    DCHECK(connection_->areStreamsPaused());
    connection_->setStreamsPaused(false);

    auto& streams = connection_->getStreams();
    for (auto it = streams.begin(); it != streams.end(); it++) {
      folly::variant_match(
          it->second,
          [](const std::unique_ptr<RocketStreamClientCallback>& stream) {
            stream->handleResumedByConnection();
          },
          [](const auto&) {
            // Only handle stream callbacks - ignore sinks/channels
          });
    }
  }

  /**
   * Schedules a timeout for stream starvation protection.
   * ONLY for streaming timeouts, not sink timeouts.
   * Delegates to ConnectionAdapter to avoid concrete type dependencies.
   */
  void scheduleStreamTimeout(void* timeoutCallback) {
    connection_->scheduleStreamTimeout(timeoutCallback);
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
