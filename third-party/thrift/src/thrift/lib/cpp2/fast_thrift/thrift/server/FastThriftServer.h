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
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/interface/debug/DebugServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/interface/monitor/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/interface/status/StatusServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftChannelServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServerRegistry.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapterFactory.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>

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
   * Attach an additional monitoring/debug handler. Methods on the monitoring
   * handler are dispatched on the same connection as the user handler;
   * routing is by method name with the user handler winning on conflict
   * (mirrors ThriftServer::setMonitoringInterface). Must be called before
   * start()/serve().
   *
   * The handler must derive from fast_thrift::MonitoringServerInterface — a
   * marker base that exists purely as a type-system guardrail to prevent
   * accidentally passing a user-facing handler here.
   *
   */
  void setMonitoringInterface(
      std::shared_ptr<fast_thrift::MonitoringServerInterface> handler);

  /**
   * Attach a Status handler. Methods on the status handler are dispatched
   * on the same connection as the user handler; routing is by method name
   * with the user handler winning on conflict (mirrors
   * ThriftServer::setStatusInterface). Must be called before
   * start()/serve().
   *
   * Tupperware health checks call `getStatus()` on this interface.
   *
   * The handler must derive from fast_thrift::StatusServerInterface — a
   * marker base that exists purely as a type-system guardrail.
   */
  void setStatusInterface(
      std::shared_ptr<fast_thrift::StatusServerInterface> handler);

  /**
   * Attach a Debug handler. Methods on the debug handler are dispatched
   * on the same connection as the user handler; routing is by method name
   * with the user handler winning on conflict (mirrors
   * ThriftServer::setDebugInterface). Must be called before
   * start()/serve().
   *
   * thriftdbg's `sendRequest`, `getServerDbgInfo`, and `info` TUI call
   * into this interface.
   *
   * The handler must derive from fast_thrift::DebugServerInterface — a
   * marker base that exists purely as a type-system guardrail.
   */
  void setDebugInterface(
      std::shared_ptr<fast_thrift::DebugServerInterface> handler);

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

  /**
   * Supply an IO thread pool to use instead of constructing one from
   * config_.numIOThreads. Lets multiple servers/subsystems share the same
   * IO threads (e.g., AsyncMcServer + FastThriftServer in ucache).
   *
   *
   * The pool MUST be constructed with min == max threads (the two-arg
   * IOThreadPoolExecutor ctor) — see the default-pool construction site
   * in start() for the rationale.
   *
   * Must be called before start()/serve(). When set, config_.numIOThreads
   * is ignored.
   */
  void setIOThreadPool(std::shared_ptr<folly::IOThreadPoolExecutorBase> pool);

  /**
   * Attach a cBPF program to the SO_REUSEPORT group that replaces the
   * kernel's default 4-tuple hash selection with uniform random across
   * worker listening sockets. Mitigates per-worker pile-up when client
   * source IPs / ports are concentrated (a single hash bucket would
   * funnel most conns to one worker). Linux-only; silently no-op'd at
   * startAccepting() time on platforms where SO_ATTACH_REUSEPORT_CBPF
   * isn't available.
   *
   * Must be called before start()/serve().
   */
  void setEnableReusePortBpfSpread(bool enable);

  /**
   * Listening-socket tuning knobs (listen backlog, TCP Fast Open, max
   * accepts per event). Applied by ConnectionHandler at startAccepting
   * time on every IO thread's listening socket. When unset, defaults
   * from connection/SocketOptions.h apply.
   *
   * Must be called before start()/serve().
   */
  void setSocketOptions(connection::SocketOptions opts);

  /**
   * Per-connection accept callback. Invoked once per accepted connection
   * (after handshake completion when TLS is enabled), with a pointer to the
   * per-connection ThriftConnContext. Use this to attach embedder-owned
   * per-connection state by calling
   * `connContext->setUserData(rocket::TypeErasedPtr)` — the framework will
   * destroy that state when the connection (and any in-flight requests
   * holding the ThriftConnContext via intrusive_ptr) tear down.
   *
   * The pointer is non-null only when `FastThriftServerConfig::
   * enableRequestContext` is true. When the flag is off, the callback still
   * fires with `nullptr` so embedders can react to accept without context
   * propagation; any `setUserData` call is impossible in that case.
   *
   * The callback runs on the IO event base that owns the connection. Must
   * be set before start()/serve(). Optional — if unset, no per-connection
   * hook runs and the connection goes straight into the pipeline as-is.
   *
   * Peer address is reachable via `connContext->getPeerAddress()` when the
   * pointer is non-null.
   */
  using OnConnectionAcceptedFn =
      std::function<void(ThriftConnContext* connContext)>;
  void setOnConnectionAccepted(OnConnectionAcceptedFn cb);

  /// Start accepting connections without blocking.
  void start();

  /// Start accepting connections and block until stop() is called.
  void serve();

  /// Stop accepting new connections and shut down.
  void stop();

  /// Get the bound server address. Useful when binding to port 0. Must be
  /// called after start() — CHECK-fails otherwise. Gate with isRunning()
  /// when iterating via FastThriftServerRegistry, which exposes unstarted
  /// servers too.
  folly::SocketAddress getAddress() const;

  /// True iff start() has been called and stop() has not yet. Cheap, lock-
  /// free read of the lifecycle state — intended for debug introspection
  /// (gating accessors that require the server to be running).
  bool isRunning() const noexcept { return state_ == State::kRunning; }

  /**
   * Returns the cached ThriftServiceMetadataResponse if
   * config.enableMetadataService was set and the server has been start()ed,
   * else nullptr. Read-only handle suitable for sharing — the response is
   * built once at start() and never mutated.
   *
   * Used by debug / introspection handlers that want to expose service
   * metadata without re-deriving it. Safe to call from any thread.
   */
  std::shared_ptr<const apache::thrift::metadata::ThriftServiceMetadataResponse>
  getMetadataResponse() const noexcept {
    return metadataResponse_;
  }

  /// Snapshot of which auxiliary slots are wired (for debug introspection).
  bool hasMonitoringHandler() const noexcept {
    return static_cast<bool>(auxInterfaces_.monitoringHandler);
  }
  bool hasStatusHandler() const noexcept {
    return static_cast<bool>(auxInterfaces_.statusHandler);
  }
  bool hasDebugHandler() const noexcept {
    return static_cast<bool>(auxInterfaces_.debugHandler);
  }

 private:
  using ServerConnectionManager = connection::ConnectionManager;

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
   * Per-connection thrift-layer state when the tail is a single generated
   * FastSvAppAdapter. Owns the adapter, its pipeline, and the buffer
   * allocator the pipeline uses. Lifetime is the server's; entries are
   * removed when the adapter's close callback fires.
   */
  struct SimpleConnection {
    ThriftServerAppAdapter::Ptr adapter;
    std::unique_ptr<server::ThriftServerTransportAdapter> transportAdapter;
    channel_pipeline::PipelineImpl::Ptr pipeline;
    std::unique_ptr<channel_pipeline::SimpleBufferAllocator> allocator;
  };

  /**
   * Per-connection thrift-layer state when the tail is a composite fronting
   * multiple user adapters. The composite borrows raw T* into its children,
   * so children must outlive it. Member dtors run in reverse declaration
   * order: `children` is declared first → destroyed last.
   */
  struct CompositeConnection {
    std::vector<ThriftServerAppAdapter::Ptr> children;
    ThriftServerCompositeAppAdapter::Ptr adapter;
    std::unique_ptr<server::ThriftServerTransportAdapter> transportAdapter;
    channel_pipeline::PipelineImpl::Ptr pipeline;
    std::unique_ptr<channel_pipeline::SimpleBufferAllocator> allocator;
  };

  using ConnectionVariant = std::variant<SimpleConnection, CompositeConnection>;

  /**
   * Auxiliary interfaces like monitoring, status, debugging, etc. will live
   * here.
   */
  struct AuxiliaryInterfaces {
    std::shared_ptr<fast_thrift::MonitoringServerInterface> monitoringHandler{
        nullptr};
    std::shared_ptr<fast_thrift::StatusServerInterface> statusHandler{nullptr};
    std::shared_ptr<fast_thrift::DebugServerInterface> debugHandler{nullptr};
  };

  connection::ConnectionFactory createConnectionFactory();

  channel_pipeline::PipelineImpl::Ptr buildRocketPipeline(
      folly::EventBase* evb,
      transport::TransportHandler* transportHandler,
      rocket::server::RocketServerAppAdapter* appAdapter,
      rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
          onSetupComplete);

  void registerConnection(
      ThriftServerAppAdapter* key, SimpleConnection connection);

  void registerConnection(
      ThriftServerCompositeAppAdapter* key, CompositeConnection connection);
  void initiateConnectionDrain();

  const FastThriftServerConfig config_;
  std::shared_ptr<ThriftServerAppAdapterFactory> handler_;
  AuxiliaryInterfaces auxInterfaces_;
  // Cached ThriftServiceMetadataResponse for the user's service. Built once
  // at start() when config_.enableMetadataService is set; null otherwise.
  // Shared across every per-connection MetadataAppAdapter.
  std::shared_ptr<const apache::thrift::metadata::ThriftServiceMetadataResponse>
      metadataResponse_;
  OnConnectionAcceptedFn onConnectionAccepted_;
  std::optional<security::FizzServerCertConfig> sslConfig_;
  security::ThriftTlsConfig thriftConfig_{};
  bool enableReusePortBpfSpread_{false};
  // Listening-socket tuning. Defaults from SocketOptions.h apply unless the
  // embedder calls setSocketOptions before start().
  connection::SocketOptions socketOptions_{};
  // IO thread pool. Either embedder-supplied via setIOThreadPool or
  // constructed in start() from config_.numIOThreads. Released on
  // destruction; the pool's own dtor joins when the last ref drops.
  std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool_;
  ServerConnectionManager::Ptr connectionManager_;
  channel_pipeline::SimpleBufferAllocator rocketAllocator_;
  folly::Synchronized<std::unordered_map<void*, ConnectionVariant>>
      thriftConnections_;
  folly::Baton<> stopBaton_;
  folly::Baton<> connectionsDrainedBaton_;
  std::atomic<bool> drainingConnections_{false};
  // Guards state_ and serializes lifecycle transitions so that stop()
  // observes the connectionManager_ assignment from start() with a proper
  // happens-before. Without this, TSAN reports a race when stop() runs on a
  // different thread from serve().
  std::mutex lifecycleMutex_;
  State state_{State::kNotStarted};
  // Process-wide registration. Mutable nowhere — declared last so it is
  // destroyed first (its destructor blocks until any in-flight
  // forEachServer callback that observed this server returns, after which
  // the rest of FastThriftServer can tear down without UAF risk).
  instrumentation::ServerTracker tracker_{
      instrumentation::kFastThriftServerTrackerKey, *this};
};

} // namespace apache::thrift::fast_thrift::thrift
