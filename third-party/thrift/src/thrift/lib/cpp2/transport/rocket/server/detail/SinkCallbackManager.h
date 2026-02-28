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

#include <chrono>

#include <thrift/lib/cpp2/transport/rocket/Types.h>

// Forward declarations to avoid circular dependencies
namespace apache::thrift::rocket {
class RocketSinkClientCallback;
template <typename>
class ConnectionAdapter;
} // namespace apache::thrift::rocket

namespace apache::thrift::rocket {

/**
 * Manages the lifecycle and operations of RocketSinkClientCallback instances.
 * This class handles SINK/CHANNEL-ONLY functionality (REQUEST_CHANNEL).
 *
 * SCOPE LIMITATION: This class handles ONLY RocketSinkClientCallback,
 * not RocketStreamClientCallback or any stream functionality.
 */
template <typename ConnectionT, template <typename> class ConnectionAdapter>
class SinkCallbackManager {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit SinkCallbackManager(Connection* connection) noexcept
      : connection_(connection) {}

  /**
   * Creates a new sink callback for REQUEST_CHANNEL handling.
   * Returns true if successful, false if streamId is already in use.
   * Delegates to ConnectionAdapter to avoid circular dependencies.
   */
  bool createSinkClientCallback(StreamId streamId) {
    return connection_->createSinkClientCallback(streamId);
  }

  /**
   * Frees a sink callback and performs cleanup.
   * ONLY handles RocketSinkClientCallback cleanup.
   * Manages inflightSinkFinalResponses_ counter.
   *
   * @param streamId The stream ID to free
   * @param markRequestComplete Whether to mark the request as complete
   */
  void freeSink(StreamId streamId, bool markRequestComplete) {
    connection_->freeSink(streamId, markRequestComplete);
  }

  /**
   * Schedules chunk timeout for sink payload processing.
   * ONLY for sink chunk timeouts, not stream timeouts.
   * Different from stream starvation timeouts - this is per-payload timeout.
   */
  void scheduleSinkTimeout(
      void* timeoutCallback, std::chrono::milliseconds timeout) {
    connection_->scheduleSinkTimeout(timeoutCallback, timeout);
  }

  /**
   * Increment inflight final response counter when sink completes.
   * Critical for connection cleanup - tracks pending final responses.
   */
  void incInflightFinalResponse() { connection_->incInflightFinalResponse(); }

  /**
   * Decrement inflight final response counter when final response sent.
   * May trigger connection cleanup if counter reaches zero during shutdown.
   */
  void decInflightFinalResponse() { connection_->decInflightFinalResponse(); }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
