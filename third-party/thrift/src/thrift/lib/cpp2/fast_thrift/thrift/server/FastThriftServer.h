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

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftChannelServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapterFactory.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * FastThriftServer — a standalone server that uses the fast_thrift pipeline
 * to serve Thrift RPCs through a generated FastSvAppAdapter handler (i.e.
 * services annotated with @cpp.FastServer).
 *
 * This is the "fast handler" variant. For the legacy AsyncProcessorFactory
 * path see FastThriftChannelServer in this directory.
 *
 * Architecture per accepted connection:
 *
 *   Rocket pipeline (owned by RocketServerConnection):
 *     TransportHandler
 *       -> FrameLengthParserHandler
 *       -> FrameLengthEncoderHandler
 *       -> RocketServerFrameCodecHandler
 *       -> RocketServerSetupFrameHandler
 *       -> RocketServerRequestResponseFrameHandler
 *       -> RocketServerStreamStateHandler
 *       -> RocketServerAppAdapter
 *
 *   Thrift pipeline (owned by FastConnection):
 *     ThriftServerTransportAdapter -> <Service>FastSvAppAdapter
 *
 * The user supplies a generated ServiceFastHandler<Service> via setInterface;
 * the server asks the factory for a fresh app adapter per connection
 * (handler lifetime is shared via std::shared_ptr).
 *
 * Usage:
 *   auto handler = std::make_shared<MyServiceImpl>();   // : public
 * MyServiceFastHandler FastThriftServerConfig config;
 *   config.address.setFromLocalPort(5001);
 *   config.numIOThreads = 8;
 *
 *   FastThriftServer server(config);
 *   server.setInterface(handler);   // implicit upcast to
 *                                   // ThriftServerAppAdapterFactory
 *   server.serve();                 // Blocks until stop() is called.
 */
class FastThriftServer {
 public:
  explicit FastThriftServer(FastThriftServerConfig config);
  ~FastThriftServer();

  FastThriftServer(const FastThriftServer&) = delete;
  FastThriftServer& operator=(const FastThriftServer&) = delete;
  FastThriftServer(FastThriftServer&&) = delete;
  FastThriftServer& operator=(FastThriftServer&&) = delete;

  /**
   * Attach the generated handler. Must be called before start()/serve().
   * User passes shared_ptr<MyHandler> — implicit upcast to
   * ThriftServerAppAdapterFactory.
   */
  void setInterface(std::shared_ptr<ThriftServerAppAdapterFactory> handler);

  /**
   * Configure TLS. After this is called, every accepted connection is wrapped
   * in a fizz::server::AsyncFizzServer; the connection factory only sees
   * fully-handshaked transports. Must be called before start()/serve().
   */
  void setSSLConfig(security::FizzServerCertConfig cfg);

  /**
   * Configure thrift-extension knobs negotiated during the fizz handshake
   * (StopTLS, params negotiation, etc.). Must be called before start()/serve().
   */
  void setThriftConfig(security::ThriftTlsConfig cfg);

  /// Start accepting connections without blocking.
  void start();

  /// Start accepting connections and block until stop() is called.
  void serve();

  /// Stop accepting new connections and shut down.
  void stop();

  /// Get the bound server address. Useful when binding to port 0.
  folly::SocketAddress getAddress() const;

 private:
  using ServerConnectionManager = rocket::server::connection::ConnectionManager;

  // Lifecycle states. Transitions are linear: kNotStarted → kRunning →
  // kStopped. start() and stop() are idempotent — calling either outside
  // its expected source state is a no-op. All transitions and reads are
  // serialized by lifecycleMutex_.
  enum class State : uint8_t {
    kNotStarted,
    kRunning,
    kStopped,
  };

  /**
   * Per-connection thrift-layer state.
   * Owns the per-connection app adapter, its pipeline, and the buffer
   * allocator the pipeline uses. Lifetime is server's; entries are
   * removed when the adapter's close callback fires.
   */
  struct FastConnection {
    std::unique_ptr<
        ThriftServerAppAdapter,
        folly::DelayedDestruction::Destructor>
        adapter;
    std::unique_ptr<server::ThriftServerTransportAdapter> transportAdapter;
    channel_pipeline::PipelineImpl::Ptr pipeline;
    std::unique_ptr<channel_pipeline::SimpleBufferAllocator> allocator =
        std::make_unique<channel_pipeline::SimpleBufferAllocator>();
  };

  rocket::server::connection::ConnectionFactory createConnectionFactory();

  channel_pipeline::PipelineImpl::Ptr buildRocketPipeline(
      folly::EventBase* evb,
      transport::TransportHandler* transportHandler,
      rocket::server::RocketServerAppAdapter* appAdapter,
      rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
          onSetupComplete);

  void registerConnection(
      ThriftServerAppAdapter* key, FastConnection connection);

  const FastThriftServerConfig config_;
  std::shared_ptr<ThriftServerAppAdapterFactory> handler_;
  std::optional<security::FizzServerCertConfig> sslConfig_;
  security::ThriftTlsConfig thriftConfig_{};
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  ServerConnectionManager::Ptr connectionManager_;
  channel_pipeline::SimpleBufferAllocator rocketAllocator_;
  folly::Synchronized<
      std::unordered_map<ThriftServerAppAdapter*, FastConnection>>
      thriftConnections_;
  folly::Baton<> stopBaton_;
  // Guards state_ and serializes lifecycle transitions so that stop()
  // observes the connectionManager_ assignment from start() with a proper
  // happens-before. Without this, TSAN reports a race when stop() runs on a
  // different thread from serve().
  std::mutex lifecycleMutex_;
  State state_{State::kNotStarted};
};

} // namespace apache::thrift::fast_thrift::thrift
