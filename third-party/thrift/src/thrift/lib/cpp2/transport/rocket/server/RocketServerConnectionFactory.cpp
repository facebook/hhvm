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

#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionFactory.h>

#include <stdexcept>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>

THRIFT_FLAG_DEFINE_bool(rocket_use_factored_server_connection, false);

namespace apache::thrift::rocket {

std::unique_ptr<IRocketServerConnection> RocketServerConnectionFactory::create(
    folly::AsyncTransport::UniquePtr socket,
    std::unique_ptr<RocketServerHandler> frameHandler,
    MemoryTracker& ingressMemoryTracker,
    MemoryTracker& egressMemoryTracker,
    StreamMetricCallback& streamMetricCallback,
    const IRocketServerConnection::Config& cfg) {
  if (THRIFT_FLAG(rocket_use_factored_server_connection)) {
    throw std::runtime_error(
        "RefactoredRocketServerConnection implementation is not yet available. "
        "This flag is for prework only. Please set "
        "rocket_use_factored_server_connection=false");
  }

  // Convert Config from interface type to concrete type
  RocketServerConnection::Config rocketCfg;
  rocketCfg.socketWriteTimeout = cfg.socketWriteTimeout;
  rocketCfg.streamStarvationTimeout = cfg.streamStarvationTimeout;
  rocketCfg.writeBatchingInterval = cfg.writeBatchingInterval;
  rocketCfg.writeBatchingSize = cfg.writeBatchingSize;
  rocketCfg.writeBatchingByteSize = cfg.writeBatchingByteSize;
  rocketCfg.egressBufferBackpressureThreshold =
      cfg.egressBufferBackpressureThreshold;
  rocketCfg.egressBufferBackpressureRecoveryFactor =
      cfg.egressBufferBackpressureRecoveryFactor;
  rocketCfg.socketOptions = cfg.socketOptions;
  rocketCfg.parserAllocator = cfg.parserAllocator;
  rocketCfg.parserStrategy = cfg.parserStrategy;

  auto* connection = new RocketServerConnection(
      std::move(socket),
      std::move(frameHandler),
      ingressMemoryTracker,
      egressMemoryTracker,
      streamMetricCallback,
      rocketCfg);

  return std::unique_ptr<IRocketServerConnection>(connection);
}

} // namespace apache::thrift::rocket
