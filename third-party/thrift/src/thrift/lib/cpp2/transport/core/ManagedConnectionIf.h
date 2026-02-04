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
#include <cstdint>
#include <vector>

#include <folly/SocketAddress.h>

#include <wangle/acceptor/ManagedConnection.h>

namespace apache::thrift {

/**
 * Information about an active interaction (tile) on a connection.
 */
struct InteractionInfo {
  int64_t interactionId{0};
  std::chrono::steady_clock::time_point creationTime{};
  size_t refCount{0};
};

/**
 * A companion to wangle::ManagedConnection exposing more information about the
 * connection.
 */
class ManagedConnectionIf : public wangle::ManagedConnection {
 public:
  ManagedConnectionIf() {}

  virtual size_t getNumActiveRequests() const = 0;
  virtual size_t getNumPendingWrites() const = 0;

  /**
   * Returns information about all active interactions on this connection.
   * Default implementation returns an empty vector for non-Rocket connections.
   */
  virtual std::vector<InteractionInfo> getInteractionSnapshots() const {
    return {};
  }

  ~ManagedConnectionIf() override = default;
};

} // namespace apache::thrift
