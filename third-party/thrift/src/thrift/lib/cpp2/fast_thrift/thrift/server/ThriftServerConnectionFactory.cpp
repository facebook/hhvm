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

#include <utility>

#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/IntervalBatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/MetadataAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionCloseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionContextHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerRequestContextHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerRequestHeadersHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/WriteBufferBackpressureHandler.h>

namespace apache::thrift::fast_thrift::thrift::server {

namespace {
using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::SimpleBufferAllocator;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(batching_frame_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(frame_codec_handler);
HANDLER_TAG(frame_defragmentation_handler);
HANDLER_TAG(frame_fragmentation_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);
HANDLER_TAG(thrift_server_request_context_handler);
HANDLER_TAG(thrift_server_connection_context_handler);
HANDLER_TAG(thrift_server_request_headers_handler);
HANDLER_TAG(thrift_server_connection_close_handler);
HANDLER_TAG(write_buffer_backpressure_handler);
} // namespace

ThriftServerConnectionFactory::ThriftServerConnectionFactory(
    ThriftServerConnectionFactoryConfig config)
    : config_(std::move(config)),
      needsComposite_(
          static_cast<bool>(config_.monitoringHandler) ||
          static_cast<bool>(config_.statusHandler) ||
          static_cast<bool>(config_.debugHandler) ||
          static_cast<bool>(config_.metadataResponse)) {
  CHECK(config_.handler)
      << "ThriftServerConnectionFactory requires a non-null handler";
}

ThriftServerConnection ThriftServerConnectionFactory::getConnection(
    folly::AsyncTransport::UniquePtr socket) {
  // Per-connection context — only built when enableRequestContext is set.
  // When unset, the thrift pipeline below skips the context-propagation
  // handlers and the embedder accept hook (wired at the connection-layer
  // ConnectionAcceptCallbackHandler) receives a null connContext.
  boost::intrusive_ptr<ThriftConnContext> connContext;
  if (config_.enableRequestContext) {
    // Snapshot peer address before consuming the socket. After TLS this
    // reflects the post-handshake peer.
    folly::SocketAddress peerAddress;
    try {
      socket->getPeerAddress(&peerAddress);
    } catch (const std::exception& ex) {
      XLOG(WARN) << "Failed to read peer address on accept: " << ex.what();
    }
    connContext.reset(new ThriftConnContext());
    connContext->setPeerAddress(peerAddress);
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

ThriftServerConnection ThriftServerConnectionFactory::buildSimpleConnection(
    folly::AsyncTransport::UniquePtr socket,
    boost::intrusive_ptr<ThriftConnContext> connContext) {
  ThriftServerConnection::SimpleTail tail{
      .adapter = config_.handler->getAppAdapter(config_.handler)};
  auto* tailAdapter = tail.adapter.get();
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
  if (config_.monitoringHandler) {
    tail.children.push_back(
        config_.monitoringHandler->getAppAdapter(config_.monitoringHandler));
  }
  if (config_.statusHandler) {
    tail.children.push_back(
        config_.statusHandler->getAppAdapter(config_.statusHandler));
  }
  if (config_.debugHandler) {
    tail.children.push_back(
        config_.debugHandler->getAppAdapter(config_.debugHandler));
  }
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
  auto transportHandler =
      transport::TransportHandler::create(std::move(socket));

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
      evb,
      transportHandler.get(),
      rocketConn.appAdapter.get(),
      [transportAdapterPtr](
          const rocket::server::handler::SetupParameters& p) noexcept {
        transportAdapterPtr->setMetadataProtocol(p.metadataProtocol);
      });
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
  using CloseHandler =
      ThriftServerConnectionCloseHandler<channel_pipeline::detail::ContextImpl>;
  using WriteBufferHandler =
      WriteBufferBackpressureHandler<channel_pipeline::detail::ContextImpl>;
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
  auto thriftPipeline = thriftPipelineBuilder.build();
  transportAdapterPtr->setPipeline(thriftPipeline.get());
  tailAdapter->setPipeline(thriftPipeline.get());
  conn.thriftPipeline = std::move(thriftPipeline);

  return conn;
}

PipelineImpl::Ptr ThriftServerConnectionFactory::buildRocketPipeline(
    folly::EventBase* evb,
    transport::TransportHandler* transportHandler,
    rocket::server::RocketServerAppAdapter* appAdapter,
    rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
        onSetupComplete) {
  return PipelineBuilder<
             transport::TransportHandler,
             rocket::server::RocketServerAppAdapter,
             SimpleBufferAllocator>()
      .setEventBase(evb)
      .setHead(transportHandler)
      .setTail(appAdapter)
      .setAllocator(&rocketAllocator_)
      .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
          frame_length_parser_handler_tag)
      .addNextOutbound<frame::write::handler::IntervalBatchingFrameHandler>(
          batching_frame_handler_tag, config_.batchingConfig)
      .addNextOutbound<frame::write::handler::FrameLengthEncoderHandler>(
          frame_length_encoder_handler_tag)
      .addNextDuplex<frame::handler::FrameCodecHandler>(frame_codec_handler_tag)
      .addNextInbound<frame::read::handler::FrameDefragmentationHandler>(
          frame_defragmentation_handler_tag)
      .addNextOutbound<frame::write::handler::FrameFragmentationHandler>(
          frame_fragmentation_handler_tag, config_.fragmentationConfig)
      .addNextDuplex<
          rocket::server::handler::RocketServerMessageMarshalHandler>(
          rocket_server_message_marshal_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerSetupFrameHandler>(
          server_setup_frame_handler_tag, std::move(onSetupComplete))
      .addNextDuplex<
          rocket::server::handler::RocketServerRequestResponseHandler>(
          server_request_response_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
          server_stream_state_handler_tag)
      .build();
}

} // namespace apache::thrift::fast_thrift::thrift::server
