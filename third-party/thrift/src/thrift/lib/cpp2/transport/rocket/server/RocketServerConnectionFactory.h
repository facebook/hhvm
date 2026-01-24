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

#include <memory>

#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/server/MemoryTracker.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>

THRIFT_FLAG_DECLARE_bool(rocket_use_factored_server_connection);

namespace apache::thrift::rocket {

/**
 * Factory class for creating RocketServerConnection instances.
 * Provides version switching capability through thrift flags.
 */
class RocketServerConnectionFactory {
 public:
  /**
   * Creates a RocketServerConnection instance based on the configured version.
   *
   * @param socket The transport socket
   * @param frameHandler The frame handler for processing requests
   * @param ingressMemoryTracker Tracker for ingress memory usage
   * @param egressMemoryTracker Tracker for egress memory usage
   * @param cfg Configuration for the connection
   * @return Unique pointer to the created connection
   */
  static std::unique_ptr<IRocketServerConnection> create(
      folly::AsyncTransport::UniquePtr socket,
      std::unique_ptr<RocketServerHandler> frameHandler,
      MemoryTracker& ingressMemoryTracker,
      MemoryTracker& egressMemoryTracker,
      const IRocketServerConnection::Config& cfg);
};

} // namespace apache::thrift::rocket
