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

#include <stdint.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <thrift/lib/cpp2/transport/core/ClientConnectionIf.h>
#include <thrift/lib/cpp2/transport/util/ConnectionThread.h>

namespace apache {
namespace thrift {

/**
 * Manages a pool of "ClientConnectionIf" objects, each one on its own
 * event base (thread) and offers them for use in a round-robin
 * fashion.
 */
class ConnectionManager {
 public:
  // Returns a singleton instance of this factory.
  static std::shared_ptr<ConnectionManager> getInstance();

  ConnectionManager();

  ~ConnectionManager() = default;

  // Returns a connection that may be used to talk to a server at
  // "addr:port".
  std::shared_ptr<ClientConnectionIf> getConnection(
      const std::string& addr, uint16_t port);

 private:
  std::vector<std::unique_ptr<ConnectionThread>> threads_;
  std::atomic_int nextThreadToUse_;
};

} // namespace thrift
} // namespace apache
