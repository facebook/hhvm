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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerConnectionFactory.h>

#include <memory>
#include <utility>

#include <folly/io/IOBuf.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BackpressurePolicy.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FragmentCompletionTracker.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/IntervalBatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/handler/RocketMetricsHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/RocketServerEventFactory.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerKeepAliveHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerWriteCompletionHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/handler/ThriftMetricsHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/SetupResponseBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/MetadataAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerChecksumHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionCloseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionContextHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerRequestContextHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerRequestHeadersHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerSetupHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/WriteBufferBackpressureHandler.h>

namespace apache::thrift::fast_thrift::thrift::server {

namespace {
using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::SimpleBufferAllocator;

// The batcher always carries the write-completion tracker. The transport fires
// TransportWriteComplete on every write whatever the configuration, and the
// tracker is the only thing that turns those into the per-batch
// RocketWriteComplete the layer above consumes; without it the transport's
// events reach an empty subscriber list. Backpressure participation is the one
// axis that varies. Both bind the rocket event space through the tracker's own
// EventId, so the batcher still hears FlushWrites either way.
using ServerBatchingFrameHandler =
    frame::write::handler::IntervalBatchingFrameHandlerT<
        frame::write::handler::WriteCompletionTrackerT<
            rocket::server::RocketServerEventFactory>>;
using ServerBatchingFrameHandlerNoBackpressure =
    frame::write::handler::IntervalBatchingFrameHandlerT<
        frame::write::handler::WriteCompletionTrackerT<
            rocket::server::RocketServerEventFactory>,
        frame::write::handler::BackpressureDisabled>;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(batching_frame_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(frame_codec_handler);
HANDLER_TAG(frame_defragmentation_handler);
HANDLER_TAG(frame_fragmentation_handler);
HANDLER_TAG(server_write_completion_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_keepalive_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);
HANDLER_TAG(thrift_server_request_context_handler);
HANDLER_TAG(thrift_server_connection_context_handler);
HANDLER_TAG(thrift_server_request_headers_handler);
HANDLER_TAG(thrift_server_checksum_handler);
HANDLER_TAG(thrift_server_connection_close_handler);
HANDLER_TAG(write_buffer_backpressure_handler);
HANDLER_TAG(thrift_server_setup_handler);
} // namespace

ThriftServerConnectionFactory::ThriftServerConnectionFactory(
    ThriftServerConnectionFactoryConfig config)
    : config_(std::move(config)),
      needsComposite_(
          static_cast<bool>(config_.monitoringHandler) ||
          static_cast<bool>(config_.statusHandler) ||
          static_cast<bool>(config_.debugHandler) ||
          static_cast<bool>(config_.securityHandler) ||
          static_cast<bool>(config_.metadataResponse)) {
  CHECK(config_.handler)
      << "ThriftServerConnectionFactory requires a non-null handler";
}

ThriftServerConnection ThriftServerConnectionFactory::getConnection(
    folly::AsyncTransport::UniquePtr socket,
    const folly::SocketAddress& clientAddr,
    const std::shared_ptr<const connection::PeerSecurityInfo>& peerSecurity) {
  // Per-connection context — only built when enableRequestContext is set.
  // When unset, the thrift pipeline below skips the context-propagation
  // handlers and the embedder accept hook (wired at the connection-layer
  // ConnectionAcceptCallbackHandler) receives a null connContext.
  boost::intrusive_ptr<ThriftConnContext> connContext;
  if (config_.enableRequestContext) {
    connContext.reset(new ThriftConnContext());
    connContext->setPeerAddress(clientAddr);
    if (peerSecurity != nullptr) {
      connContext->setPeerCertificate(peerSecurity->peerCertificate);
      connContext->setSecurityProtocol(peerSecurity->securityProtocol);
    }
  }

  auto conn = needsComposite_
      ? buildCompositeConnection(std::move(socket), connContext)
      : buildSimpleConnection(std::move(socket), connContext);

  // Expose the context on the connection so the connection-layer accept
  // callback can reach it post-construction.
  conn.connContext = std::move(connContext);

  // Note: the connection is fully wired but inert. Reading is started
  // separately via ThriftServerConnection::start() once the connection
  // layer has run its accept-time setup (e.g. onConnectionAccepted hook,
  // registration in the connection-manager map). Starting here would race
  // those steps: setReadCB can synchronously drain pre-received bytes
  // (post-StopTLS handoff) and dispatch the first request before the
  // accept hook has populated per-connection state.
  return conn;
}

void ThriftServerConnectionFactory::attachCPUExecutor(
    ThriftServerAppAdapter& adapter) const {
  if (config_.cpuExecutor) {
    adapter.setCPUExecutor(config_.cpuExecutor);
  }
}

ThriftServerConnection ThriftServerConnectionFactory::buildSimpleConnection(
    folly::AsyncTransport::UniquePtr socket,
    boost::intrusive_ptr<ThriftConnContext> connContext) {
  ThriftServerConnection::SimpleTail tail{
      .adapter = config_.handler->getAppAdapter(config_.handler)};
  auto* tailAdapter = tail.adapter.get();
  attachCPUExecutor(*tailAdapter);
  return buildConnectionImpl<ThriftServerAppAdapter>(
      std::move(socket), std::move(tail), tailAdapter, std::move(connContext));
}

ThriftServerConnection ThriftServerConnectionFactory::buildCompositeConnection(
    folly::AsyncTransport::UniquePtr socket,
    boost::intrusive_ptr<ThriftConnContext> connContext) {
  // Build the composite tail: user adapter + each wired aux + metadata.
  // children must outlive the composite (composite borrows raw T* into
  // them); ThriftServerConnection::CompositeTail field declaration order
  // guarantees this.
  ThriftServerConnection::CompositeTail tail;
  tail.children.push_back(config_.handler->getAppAdapter(config_.handler));
  attachCPUExecutor(*tail.children.back());
  // Aux interfaces offload like the user handler. Methods that must stay on
  // the EventBase are pinned per-method in their IDLs via
  // @cpp.ProcessInEbThreadUnsafe, not by withholding the executor here.
  if (config_.monitoringHandler) {
    tail.children.push_back(
        config_.monitoringHandler->getAppAdapter(config_.monitoringHandler));
    attachCPUExecutor(*tail.children.back());
  }
  if (config_.statusHandler) {
    tail.children.push_back(
        config_.statusHandler->getAppAdapter(config_.statusHandler));
    attachCPUExecutor(*tail.children.back());
  }
  if (config_.debugHandler) {
    tail.children.push_back(
        config_.debugHandler->getAppAdapter(config_.debugHandler));
    attachCPUExecutor(*tail.children.back());
  }
  if (config_.securityHandler) {
    tail.children.push_back(
        config_.securityHandler->getAppAdapter(config_.securityHandler));
    attachCPUExecutor(*tail.children.back());
  }
  // Deliberately not offloaded: MetadataAppAdapter is hand-written rather
  // than generated, so it never consults cpuExecutor() and completes through
  // the EventBase-only writeResponse overload. Attaching an executor would
  // compile and do nothing. Its body only re-serializes an immutable
  // prebuilt response, so there is nothing to move off the EventBase.
  if (config_.metadataResponse) {
    tail.children.push_back(
        ThriftServerAppAdapter::Ptr{
            new MetadataAppAdapter(config_.metadataResponse)});
  }
  tail.adapter = ThriftServerCompositeAppAdapter::Ptr{
      new ThriftServerCompositeAppAdapter()};
  for (auto& child : tail.children) {
    tail.adapter->addChild(child.get());
  }
  auto* compositeAdapter = tail.adapter.get();
  return buildConnectionImpl<ThriftServerCompositeAppAdapter>(
      std::move(socket),
      std::move(tail),
      compositeAdapter,
      std::move(connContext));
}

template <typename TailAdapter>
ThriftServerConnection ThriftServerConnectionFactory::buildConnectionImpl(
    folly::AsyncTransport::UniquePtr socket,
    std::variant<
        std::monostate,
        ThriftServerConnection::SimpleTail,
        ThriftServerConnection::CompositeTail> tail,
    TailAdapter* tailAdapter,
    boost::intrusive_ptr<ThriftConnContext> connContext) {
  auto* evb = socket->getEventBase();

  // Accept runs on the IO thread that will own this connection, which is the
  // thread holding the shard these counts belong in — both to look it up here
  // and to increment it from the pipeline later.
  ServerStatsShard* statsShard = nullptr;
  if (config_.stats) {
    DCHECK(evb->isInEventBaseThread());
    statsShard = &config_.stats->currentThreadShard();
  }

  auto transportHandler =
      rocket::server::RocketServerTransportHandler::create(std::move(socket));

  ThriftServerConnection conn;
  conn.tail = std::move(tail);

  // Construct the transport adapter early around a fresh (empty) rocket
  // connection so the rocket pipeline's SETUP callback below can capture a
  // stable pointer into it. The rocket connection's appAdapter is
  // default-initialized; transportHandler / pipeline are populated after
  // buildRocketPipeline runs.
  conn.thriftTransportAdapter = std::make_unique<ThriftServerTransportAdapter>(
      std::make_unique<rocket::server::RocketServerConnection>());
  auto& rocketConn = conn.thriftTransportAdapter->rocketConnection();
  auto* transportAdapterPtr = conn.thriftTransportAdapter.get();

  auto rocketPipeline = buildRocketPipeline(
      evb, transportHandler.get(), rocketConn.appAdapter.get(), statsShard);
  rocketConn.appAdapter->setPipeline(rocketPipeline.get());
  transportHandler->setPipeline(rocketPipeline.get());

  if (config_.zeroCopyThreshold > 0) {
    if (!transportHandler->setZeroCopy(true)) {
      XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
    }
    transportHandler->setZeroCopyEnableThreshold(config_.zeroCopyThreshold);
  }
  rocketConn.transportHandler = std::move(transportHandler);
  rocketConn.pipeline = std::move(rocketPipeline);

  // Thrift pipeline templated on the tail adapter type. For the simple case
  // this works because generated FastSvAppAdapter subclasses only populate
  // dispatch_ via addMethodHandler in their ctor and don't override base
  // methods; for the composite case the typed tail also fans setPipeline
  // out to every child. When enableRequestContext is set, wire the
  // per-connection context handlers so each request's ThriftRequestContext
  // is populated with the ThriftConnContext.
  using ReqCtxHandler =
      ThriftServerRequestContextHandler<channel_pipeline::detail::ContextImpl>;
  using ConnCtxHandler = ThriftServerConnectionContextHandler<
      channel_pipeline::detail::ContextImpl>;
  using ReqHeadersHandler =
      ThriftServerRequestHeadersHandler<channel_pipeline::detail::ContextImpl>;
  using ChecksumHandler =
      ThriftServerChecksumHandler<channel_pipeline::detail::ContextImpl>;
  using CloseHandler =
      ThriftServerConnectionCloseHandler<channel_pipeline::detail::ContextImpl>;
  using WriteBufferHandler =
      WriteBufferBackpressureHandler<channel_pipeline::detail::ContextImpl>;
  using SetupHandler =
      ThriftServerSetupHandler<channel_pipeline::detail::ContextImpl>;
  PipelineBuilder<
      ThriftServerTransportAdapter,
      TailAdapter,
      SimpleBufferAllocator,
      ThriftServerEventType>
      thriftPipelineBuilder;
  thriftPipelineBuilder.setEventBase(evb)
      .setHead(transportAdapterPtr)
      .setTail(tailAdapter)
      .setAllocator(&conn.thriftAllocator);
  // Sits closest to the head so it sees every message crossing the thrift
  // layer, before any handler below can absorb or synthesize one.
  if (statsShard != nullptr) {
    thriftPipelineBuilder.template addNextDuplex<
        ThriftMetricsHandler<Direction::Server, ServerStatsShard>>(
        thrift_metrics_handler_tag, statsShard);
  }
  DCHECK(!config_.enableRequestHeaders || config_.enableRequestContext)
      << "enableRequestHeaders requires enableRequestContext; the request "
         "headers handler is skipped while enableRequestContext is off";
  if (config_.enableRequestContext) {
    thriftPipelineBuilder
        .template addNextInbound<ReqCtxHandler>(
            thrift_server_request_context_handler_tag)
        .template addNextInbound<ConnCtxHandler>(
            thrift_server_connection_context_handler_tag,
            std::move(connContext));
    // Stamps RequestRpcMetadata.otherMetadata onto each request's
    // ThriftRequestContext. Requires the context handlers above, so it is
    // nested under enableRequestContext.
    if (config_.enableRequestHeaders) {
      thriftPipelineBuilder.template addNextInbound<ReqHeadersHandler>(
          thrift_server_request_headers_handler_tag);
    }
  }
  // Validates the inbound request checksum and fills the response checksum.
  // Added after the context handlers so inbound it runs once the per-request
  // ThriftRequestContext exists (it records the algorithm there for the
  // response to echo).
  CHECK(!config_.enableChecksum || config_.enableRequestContext)
      << "enableChecksum requires enableRequestContext; the checksum handler "
         "records the response algorithm on the per-request context";
  if (config_.enableChecksum) {
    thriftPipelineBuilder.template addNextDuplex<ChecksumHandler>(
        thrift_server_checksum_handler_tag);
  }
  // Connection-close handler sits immediately upstream of the tail.
  // ThriftServerConnection::close() fires
  // ThriftServerEventType::CloseConnection through the pipeline; the handler
  // picks it up via onEvent and drives the terminal state machine.
  thriftPipelineBuilder.template addNextDuplex<CloseHandler>(
      thrift_server_connection_close_handler_tag,
      config_.drainTimeout,
      config_.reapTimeout);
  // Write-buffer handler sits between the context handlers and the drain
  // handler. Placed above drain (closer to head) so its inbound
  // Backpressure signal propagates upstream toward the transport, and
  // outbound responses from the tail traverse drain → write-buffer →
  // head.
  if (config_.enableWriteBufferBackpressure) {
    thriftPipelineBuilder.template addNextDuplex<WriteBufferHandler>(
        write_buffer_backpressure_handler_tag);
  }
  // Embedder-registered handlers go after all built-ins, in registration
  // order — the first sits closest to the head, the last immediately above
  // the tail adapter. Each factory constructs a fresh per-connection instance.
  for (const auto& factory : config_.thriftPipelineHandlerFactories) {
    thriftPipelineBuilder.addErasedHandler(factory(conn.extensionStates));
  }
  // Last before the tail, and deliberately after the embedder handlers: this
  // terminates the connection-lifecycle messages, so everything that might
  // answer one has to run first. The application tail then only ever sees
  // requests.
  thriftPipelineBuilder.template addNextDuplex<SetupHandler>(
      thrift_server_setup_handler_tag);
  auto thriftPipeline = thriftPipelineBuilder.build();
  transportAdapterPtr->setPipeline(thriftPipeline.get());
  tailAdapter->setPipeline(thriftPipeline.get());
  conn.thriftPipeline = std::move(thriftPipeline);

  return conn;
}

PipelineImpl::Ptr ThriftServerConnectionFactory::buildRocketPipeline(
    folly::EventBase* evb,
    rocket::server::RocketServerTransportHandler* transportHandler,
    rocket::server::RocketServerAppAdapter* appAdapter,
    ServerStatsShard* FOLLY_NULLABLE statsShard) {
  // addState rebinds: it returns a builder of an extended type and leaves the
  // original moved-from, so the chain up to and including it must be bound
  // here rather than continued on a pre-declared builder.
  auto builder = PipelineBuilder<
                     rocket::server::RocketServerTransportHandler,
                     rocket::server::RocketServerAppAdapter,
                     SimpleBufferAllocator,
                     rocket::server::RocketServerEventId>()
                     .setEventBase(evb)
                     .setHead(transportHandler)
                     .setTail(appAdapter)
                     .setAllocator(&rocketAllocator_)
                     .addState<rocket::RocketStreamContexts>();
  builder.addNextInbound<frame::read::handler::FrameLengthParserHandler>(
      frame_length_parser_handler_tag);
  // Batching and fragmentation are always present, but which specialization
  // is spliced depends on enableBackpressure. The no-backpressure variants
  // batch and fragment identically; they simply carry no write-ready hook, so
  // makeHandlerNode never registers them and the pipeline's writeReadyList_
  // stays empty. The choice costs one branch per connection, not per message.
  if (config_.enableBackpressure) {
    builder
        // Batcher composed with the write-completion tracker: the tracker turns
        // the transport's per-writev completions into one event per
        // rocket-frame batch. The fragmentation handler below turns that into
        // the rocket-level completion the app adapter relays up to the thrift
        // pipeline.
        .addNextOutbound<ServerBatchingFrameHandler>(
            batching_frame_handler_tag, config_.batchingConfig);
  } else {
    builder.addNextOutbound<ServerBatchingFrameHandlerNoBackpressure>(
        batching_frame_handler_tag, config_.batchingConfig);
  }
  builder
      .addNextOutbound<frame::write::handler::FrameLengthEncoderHandler>(
          frame_length_encoder_handler_tag)
      .addNextDuplex<frame::handler::FrameCodecHandler>(frame_codec_handler_tag)
      .addNextInbound<frame::read::handler::FrameDefragmentationHandler>(
          frame_defragmentation_handler_tag);
  if (config_.enableBackpressure) {
    // Fragmenter composed with the fragment-completion tracker, same as the
    // client pipeline. It records each frame's streamId on the way down — below
    // this handler the frame is serialized bytes and the stream is no longer
    // recoverable — and fans the batcher's batch event back out into one
    // completion per original frame. Paired with the batcher's tracker above;
    // a NoOp here would strand the batch events with no subscriber.
    builder.addNextOutbound<frame::write::handler::FrameFragmentationHandlerT<
        frame::write::handler::FragmentCompletionTrackerT<
            rocket::server::RocketServerEventFactory>>>(
        frame_fragmentation_handler_tag, config_.fragmentationConfig);
  } else {
    // Matches the batcher above: no tracker there means no batch events, so
    // there is nothing for a relay to forward.
    builder.addNextOutbound<
        frame::write::handler::FrameFragmentationHandlerNoBackpressure>(
        frame_fragmentation_handler_tag, config_.fragmentationConfig);
  }
  builder
      // Restates the fragmenter's frame-layer completion as the rocket layer's
      // own, so nothing above the rocket boundary subscribes to a frame event.
      .addNextDuplex<
          rocket::server::handler::RocketServerWriteCompletionHandler>(
          server_write_completion_handler_tag)
      .addNextDuplex<
          rocket::server::handler::RocketServerMessageMarshalHandler>(
          rocket_server_message_marshal_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerSetupFrameHandler>(
          server_setup_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerKeepAliveHandler>(
          server_keepalive_handler_tag)
      .addNextDuplex<
          rocket::server::handler::RocketServerRequestResponseHandler>(
          server_request_response_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
          server_stream_state_handler_tag);
  // Sits closest to the tail, so inbound it counts frames that survived
  // parsing/defragmentation and outbound it counts frames as the app emits
  // them, before batching or fragmentation can change the frame count.
  if (statsShard != nullptr) {
    builder.addNextDuplex<
        RocketMetricsHandler<Direction::Server, ServerStatsShard>>(
        rocket_metrics_handler_tag, statsShard);
  }
  return builder.build();
}

} // namespace apache::thrift::fast_thrift::thrift::server
