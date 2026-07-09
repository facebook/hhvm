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
#include <cstddef>
#include <memory>
#include <variant>

#include <boost/intrusive_ptr.hpp>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FragmentationHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/IntervalBatchingHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/interface/debug/DebugServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/interface/monitor/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/interface/status/StatusServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapterFactory.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ThriftServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * Configuration consumed by ThriftServerConnectionFactory. Captured once at
 * construction time; the factory installs the resulting handler / aux
 * interfaces / metadata into every connection it builds.
 */
struct ThriftServerConnectionFactoryConfig {
  std::shared_ptr<ThriftServerAppAdapterFactory> handler;
  std::shared_ptr<fast_thrift::MonitoringServerInterface> monitoringHandler;
  std::shared_ptr<fast_thrift::StatusServerInterface> statusHandler;
  std::shared_ptr<fast_thrift::DebugServerInterface> debugHandler;
  std::shared_ptr<const apache::thrift::metadata::ThriftServiceMetadataResponse>
      metadataResponse;
  // Per-connection MSG_ZEROCOPY threshold; 0 disables zero-copy.
  std::size_t zeroCopyThreshold{0};
  // When true, build a per-connection ThriftConnContext on accept and wire
  // the ThriftServerRequestContextHandler +
  // ThriftServerConnectionContextHandler into the thrift pipeline so each
  // request's ThriftRequestContext is populated with the ThriftConnContext.
  bool enableRequestContext{false};

  // When true, insert ThriftServerRequestHeadersHandler so each request's
  // ThriftRequestContext is populated with the inbound custom headers
  // (RequestRpcMetadata.otherMetadata). Requires enableRequestContext.
  bool enableRequestHeaders{false};

  // When true, insert WriteBufferBackpressureHandler into the thrift
  // pipeline to absorb outbound Backpressure: responses queue in a FIFO
  // when downstream is saturated and drain on onWriteReady; inbound reads
  // surface Backpressure while buffered to pause socket reads.
  bool enableWriteBufferBackpressure{false};
  // Outbound write batching. Default zero-interval flushes via LoopCallback
  // at end of each event loop iteration. Set batchingInterval > 0 to use
  // an HHWheelTimer-driven flush instead.
  frame::write::IntervalBatchingHandlerConfig batchingConfig{};

  // Outbound frame fragmentation. Splits oversized PAYLOAD / REQUEST_*
  // frames into spec-compliant fragment chains; small frames bypass.
  frame::write::FragmentationHandlerConfig fragmentationConfig{};

  // Per-connection terminal-phase deadlines passed to
  // ThriftServerConnectionCloseHandler. Defaults mirror that handler's
  // kDefaultDrainTimeout / kDefaultReapTimeout. See FastThriftServerConfig
  // for semantics.
  std::chrono::milliseconds drainTimeout{std::chrono::seconds{30}};
  std::chrono::milliseconds reapTimeout{std::chrono::seconds{60}};
};

/**
 * ThriftServerConnectionFactory builds a fully-wired ThriftServerConnection
 * (rocket pipeline owned via the transport adapter + thrift pipeline) for
 * each accepted socket. Used by ConnectionHandler to materialize and install
 * connections.
 *
 * Public API is non-templated so ConnectionHandler can store the factory by
 * value/reference without knowing the thrift tail-adapter shape. Internally,
 * a small templated helper picks the tail type (SimpleTail vs CompositeTail)
 * based on whether any auxiliary interface or metadata response is wired.
 */
class ThriftServerConnectionFactory {
 public:
  explicit ThriftServerConnectionFactory(
      ThriftServerConnectionFactoryConfig config);

  /**
   * Build a connection for `socket`. Constructs the per-connection
   * ThriftConnContext (when enableRequestContext is set), builds the rocket +
   * thrift pipelines, and fires `onConnect()` before returning.
   *
   * Satisfies the connection::ConnectionFactory concept.
   */
  ThriftServerConnection getConnection(folly::AsyncTransport::UniquePtr socket);

 private:
  // Builds the rocket pipeline (TransportHandler ... RocketServerAppAdapter).
  // Same shape as the legacy FastThriftServer::buildRocketPipeline; lives
  // here because the factory owns the shared rocket allocator.
  channel_pipeline::PipelineImpl::Ptr buildRocketPipeline(
      folly::EventBase* evb,
      transport::TransportHandler* transportHandler,
      rocket::server::RocketServerAppAdapter* appAdapter,
      rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
          onSetupComplete);

  ThriftServerConnection buildSimpleConnection(
      folly::AsyncTransport::UniquePtr socket,
      boost::intrusive_ptr<ThriftConnContext> connContext);

  ThriftServerConnection buildCompositeConnection(
      folly::AsyncTransport::UniquePtr socket,
      boost::intrusive_ptr<ThriftConnContext> connContext);

  // Shared body for the two builders above. Templated on the thrift-pipeline
  // tail type because PipelineBuilder<Head, Tail, Allocator> needs the
  // concrete tail at build time. Defined in the .cpp; implicitly instantiated
  // for ThriftServerAppAdapter and ThriftServerCompositeAppAdapter when
  // buildSimpleConnection / buildCompositeConnection call it from the same TU.
  template <typename TailAdapter>
  ThriftServerConnection buildConnectionImpl(
      folly::AsyncTransport::UniquePtr socket,
      std::variant<
          std::monostate,
          ThriftServerConnection::SimpleTail,
          ThriftServerConnection::CompositeTail> tail,
      TailAdapter* tailAdapter,
      boost::intrusive_ptr<ThriftConnContext> connContext);

  ThriftServerConnectionFactoryConfig config_;
  // Computed once from config_ in the ctor: true iff any aux interface or
  // metadata response is wired (i.e. the thrift tail must be a composite).
  bool needsComposite_;
  // Shared across all rocket pipelines built by this factory, matching the
  // pre-refactor behavior where FastThriftServer held a single
  // rocketAllocator_.
  channel_pipeline::SimpleBufferAllocator rocketAllocator_;
};

} // namespace apache::thrift::fast_thrift::thrift::server
