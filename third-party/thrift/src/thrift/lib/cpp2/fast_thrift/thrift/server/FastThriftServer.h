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

#include <atomic>
#include <unordered_map>

#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Configuration for FastThriftServer.
 */
struct FastThriftServerConfig {
  // Address to bind to.
  folly::SocketAddress address;

  // Number of IO threads. Each thread runs its own EventBase and accepts
  // connections via SO_REUSEPORT.
  uint32_t numIOThreads{1};

  // Minimum payload size in bytes for MSG_ZEROCOPY. 0 disables zero-copy.
  size_t zeroCopyThreshold{0};
};

/**
 * FastThriftServer - A standalone server that uses the fast_thrift pipeline
 * to serve Thrift RPCs.
 *
 * Wires up the full fast_thrift server pipeline:
 *   TransportHandler
 *     -> FrameLengthParserHandler
 *     -> FrameLengthEncoderHandler
 *     -> RocketServerFrameCodecHandler
 *     -> RocketServerSetupFrameHandler
 *     -> RocketServerRequestResponseFrameHandler
 *     -> RocketServerStreamStateHandler
 *     -> ThriftServerChannel (dispatches to AsyncProcessor)
 *
 * Supports request-response and oneway RPCs. Streaming and sink RPCs are
 * not yet supported.
 *
 * Usage:
 *   auto handler = std::make_shared<MyServiceHandler>();
 *   FastThriftServerConfig config;
 *   config.address.setFromLocalPort(5001);
 *   config.numIOThreads = 8;
 *
 *   FastThriftServer server(config, handler);
 *   server.serve();  // Blocks until stop() is called from another thread.
 */
class FastThriftServer {
 public:
  FastThriftServer(
      FastThriftServerConfig config,
      std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory);
  ~FastThriftServer();

  FastThriftServer(const FastThriftServer&) = delete;
  FastThriftServer& operator=(const FastThriftServer&) = delete;
  FastThriftServer(FastThriftServer&&) = delete;
  FastThriftServer& operator=(FastThriftServer&&) = delete;

  /**
   * Start accepting connections without blocking.
   */
  void start();

  /**
   * Start accepting connections and block until stop() is called.
   */
  void serve();

  /**
   * Stop accepting new connections and shut down.
   * In-flight requests on existing connections will complete before
   * channels are destroyed.
   */
  void stop();

  /**
   * Get the bound server address.
   * Useful when binding to port 0 to discover the assigned port.
   */
  folly::SocketAddress getAddress() const;

 private:
  using ServerTransportHandler = transport::TransportHandler;
  using ServerConnectionManager =
      rocket::server::connection::ConnectionManager<ServerTransportHandler>;

  rocket::server::connection::PipelineFactory<ServerTransportHandler>
  createPipelineFactory();

  channel_pipeline::PipelineImpl::Ptr buildPipeline(
      folly::EventBase* evb,
      ServerTransportHandler* transportHandler,
      ThriftServerChannel* serverChannel);

  void registerChannel(std::shared_ptr<ThriftServerChannel> channel);

  const FastThriftServerConfig config_;
  std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  ServerConnectionManager::Ptr connectionManager_;
  channel_pipeline::SimpleBufferAllocator allocator_;
  // Uses raw ThriftServerChannel* as key for O(1) lookup from close callbacks,
  // which only have a raw pointer to the channel being closed. The shared_ptr
  // value owns the channel lifetime.
  folly::Synchronized<std::unordered_map<
      ThriftServerChannel*,
      std::shared_ptr<ThriftServerChannel>>>
      serverChannels_;
  folly::Baton<> stopBaton_;
  std::atomic<bool> started_{false};
  std::atomic<bool> stopped_{false};
};

} // namespace apache::thrift::fast_thrift::thrift
