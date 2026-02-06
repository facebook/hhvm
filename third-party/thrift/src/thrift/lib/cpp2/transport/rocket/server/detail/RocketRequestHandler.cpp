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

#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketRequestHandler.h>

#include <utility>

#include <folly/lang/Assume.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketThriftRequests.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketErrorHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketRequestOrchestrator.h>

namespace apache::thrift::rocket {

namespace {
int64_t getDefaultLogSampleRatio() {
  return THRIFT_REQUEST_EVENT(success).shouldLog();
}

int64_t getDefaultLogErrorSampleRatio() {
  return THRIFT_REQUEST_EVENT(error).shouldLog();
}
} // namespace

thread_local uint32_t RocketRequestHandler::sample_{0};

RocketRequestHandler::RocketRequestHandler(
    AsyncProcessorFactory* processorFactory,
    std::shared_ptr<AsyncProcessorFactory> processorFactoryStorage,
    Cpp2Worker::PerServiceMetadata* serviceMetadata,
    std::shared_ptr<AsyncProcessor> processor,
    std::shared_ptr<concurrency::ThreadManager> threadManager,
    server::ServerConfigs* serverConfigs,
    RequestsRegistry* requestsRegistry,
    folly::EventBase* eventBase,
    Cpp2ConnContext* connContext,
    ThriftServer* server,
    Cpp2Worker* worker,
    folly::AsyncTransport* transport,
    uint32_t sampleRate,
    int32_t version,
    std::chrono::milliseconds maxResponseWriteTime,
    folly::once_flag* setupLoggingFlag,
    RocketErrorHandler* errorHandler)
    : processorFactory_(processorFactory),
      processorFactoryStorage_(std::move(processorFactoryStorage)),
      serviceMetadata_(serviceMetadata),
      processor_(std::move(processor)),
      threadManager_(std::move(threadManager)),
      serverConfigs_(serverConfigs),
      requestsRegistry_(requestsRegistry),
      eventBase_(eventBase),
      connContext_(connContext),
      server_(server),
      worker_(worker),
      transport_(transport),
      sampleRate_(sampleRate),
      version_(version),
      maxResponseWriteTime_(maxResponseWriteTime),
      setupLoggingFlag_(setupLoggingFlag),
      errorHandler_(errorHandler) {}

apache::thrift::server::TServerObserver::SamplingStatus
RocketRequestHandler::shouldSample(const transport::THeader& header) {
  bool isServerSamplingEnabled =
      (sampleRate_ > 0) && ((sample_++ % sampleRate_) == 0);

  int64_t logSampleRatio = 0;
  int64_t logErrorSampleRatio = 0;
  if (const auto& loggingContext = header.loggingContext()) {
    logSampleRatio = *loggingContext->logSampleRatio();
    logErrorSampleRatio = *loggingContext->logErrorSampleRatio();
  } else {
    // use sampling ratios from the server config if client doesn't set the
    // logging context
    logSampleRatio = getDefaultLogSampleRatio();
    logErrorSampleRatio = getDefaultLogErrorSampleRatio();
  }

  return apache::thrift::server::TServerObserver::SamplingStatus(
      isServerSamplingEnabled, logSampleRatio, logErrorSampleRatio);
}

void RocketRequestHandler::handleRequestResponse(
    RequestResponseFrame&& frame, RocketServerFrameContext&& context) {
  auto makeRequestResponse = [&](RequestRpcMetadata&& md,
                                 rocket::Payload&& debugPayload,
                                 std::shared_ptr<folly::RequestContext>&& ctx) {
    return this->makeRequestResponse(
        std::move(md),
        std::move(debugPayload),
        std::move(ctx),
        std::move(context));
  };

  handleRequestCommon(
      std::move(frame.payload()),
      std::move(makeRequestResponse),
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      context.connection());
}

void RocketRequestHandler::handleRequestFnf(
    RequestFnfFrame&& frame, RocketServerFrameContext&& context) {
  auto makeRequestFnf = [&](RequestRpcMetadata&& md,
                            rocket::Payload&& debugPayload,
                            std::shared_ptr<folly::RequestContext>&& ctx) {
    return this->makeRequestFnf(
        std::move(md),
        std::move(debugPayload),
        std::move(ctx),
        std::move(context));
  };

  handleRequestCommon(
      std::move(frame.payload()),
      std::move(makeRequestFnf),
      RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      context.connection());
}

void RocketRequestHandler::handleRequestStream(
    RequestStreamFrame&& frame,
    RocketServerFrameContext&& context,
    RocketStreamClientCallback* clientCallback) {
  auto makeRequestStream = [&](RequestRpcMetadata&& md,
                               rocket::Payload&& debugPayload,
                               std::shared_ptr<folly::RequestContext>&& ctx) {
    return this->makeRequestStream(
        std::move(md),
        std::move(debugPayload),
        std::move(ctx),
        std::move(context),
        clientCallback);
  };

  handleRequestCommon(
      std::move(frame.payload()),
      std::move(makeRequestStream),
      RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE,
      context.connection());
}

void RocketRequestHandler::handleRequestChannel(
    RequestChannelFrame&& frame,
    RocketServerFrameContext&& context,
    ChannelRequestCallbackFactory clientCallback) {
  auto makeRequestSink = [&](RequestRpcMetadata&& md,
                             rocket::Payload&& debugPayload,
                             std::shared_ptr<folly::RequestContext>&& ctx)
      -> ThriftRequestCoreUniquePtr {
    return this->makeRequestSink(
        std::move(md),
        std::move(debugPayload),
        std::move(ctx),
        std::move(context),
        clientCallback);
  };

  handleRequestCommon(
      std::move(frame.payload()),
      std::move(makeRequestSink),
      RpcKind::SINK,
      context.connection());
}

ThriftRequestCoreUniquePtr RocketRequestHandler::makeRequestResponse(
    RequestRpcMetadata&& md,
    rocket::Payload&& debugPayload,
    std::shared_ptr<folly::RequestContext> ctx,
    RocketServerFrameContext&& context) {
  // Note, we're passing connContext by reference and rely on the next
  // chain of ownership to keep it alive: ThriftServerRequestResponse
  // stores RocketServerFrameContext, which keeps refcount on
  // RocketServerConnection, which in turn keeps ThriftRocketServerHandler
  // alive, which in turn keeps connContext_ alive.
  return RequestsRegistry::makeRequest<ThriftServerRequestResponse>(
      *eventBase_,
      *serverConfigs_,
      std::move(md),
      *connContext_,
      std::move(ctx),
      *requestsRegistry_,
      std::move(debugPayload),
      std::move(context),
      version_,
      maxResponseWriteTime_);
}

ThriftRequestCoreUniquePtr RocketRequestHandler::makeRequestFnf(
    RequestRpcMetadata&& md,
    rocket::Payload&& debugPayload,
    std::shared_ptr<folly::RequestContext> ctx,
    RocketServerFrameContext&& context) {
  // Note, we're passing connContext by reference and rely on a complex
  // chain of ownership (see makeRequestResponse for detailed explanation).
  return RequestsRegistry::makeRequest<ThriftServerRequestFnf>(
      *eventBase_,
      *serverConfigs_,
      std::move(md),
      *connContext_,
      std::move(ctx),
      *requestsRegistry_,
      std::move(debugPayload),
      std::move(context),
      [keepAlive = processor_] {});
}

ThriftRequestCoreUniquePtr RocketRequestHandler::makeRequestStream(
    RequestRpcMetadata&& md,
    rocket::Payload&& debugPayload,
    std::shared_ptr<folly::RequestContext> ctx,
    RocketServerFrameContext&& context,
    RocketStreamClientCallback* clientCallback) {
  return RequestsRegistry::makeRequest<ThriftServerRequestStream>(
      *eventBase_,
      *serverConfigs_,
      std::move(md),
      *connContext_,
      std::move(ctx),
      *requestsRegistry_,
      std::move(debugPayload),
      std::move(context),
      version_,
      clientCallback,
      processor_);
}

ThriftRequestCoreUniquePtr RocketRequestHandler::makeRequestSink(
    RequestRpcMetadata&& md,
    rocket::Payload&& debugPayload,
    std::shared_ptr<folly::RequestContext> ctx,
    RocketServerFrameContext&& context,
    ChannelRequestCallbackFactory& clientCallback) {
  if (md.kind() == RpcKind::SINK) {
    return RequestsRegistry::makeRequest<ThriftServerRequestSink>(
        *eventBase_,
        *serverConfigs_,
        std::move(md),
        *connContext_,
        std::move(ctx),
        *requestsRegistry_,
        std::move(debugPayload),
        std::move(context),
        version_,
        clientCallback.create<RocketSinkClientCallback>(),
        processor_);
  } else if (md.kind() == RpcKind::BIDIRECTIONAL_STREAM) {
    return RequestsRegistry::makeRequest<ThriftServerRequestBiDi>(
        *eventBase_,
        *serverConfigs_,
        std::move(md),
        *connContext_,
        std::move(ctx),
        *requestsRegistry_,
        std::move(debugPayload),
        std::move(context),
        version_,
        clientCallback.create<RocketBiDiClientCallback>(),
        processor_);
  } else {
    if (md.kind().has_value()) {
      LOG(FATAL) << "Invalid request kind: " << static_cast<int>(*md.kind());
    } else {
      LOG(FATAL) << "Request kind is not set";
    }
  }
}

template <class F>
void RocketRequestHandler::handleRequestCommon(
    Payload&& payload,
    F&& makeRequest,
    RpcKind expectedKind,
    IRocketServerConnection& icontext) {
  // Check for injected failures (for testing/debugging)
  std::optional<RocketErrorHandler::InjectedFault> injectedFault = std::nullopt;
  if (auto injectedFailure = server_->maybeInjectFailure();
      injectedFailure != ThriftServer::InjectedFailure::NONE) {
    switch (injectedFailure) {
      case ThriftServer::InjectedFailure::NONE:
        folly::assume_unreachable();
      case ThriftServer::InjectedFailure::ERROR:
        injectedFault = RocketErrorHandler::InjectedFault::ERROR;
        break;
      case ThriftServer::InjectedFailure::DROP:
        injectedFault = RocketErrorHandler::InjectedFault::DROP;
        break;
      case ThriftServer::InjectedFailure::DISCONNECT:
        injectedFault = RocketErrorHandler::InjectedFault::DISCONNECT;
        break;
      default:
        folly::assume_unreachable();
    }
  }

  RocketRequestOrchestrator orchestrator(
      transport_,
      *connContext_,
      *setupLoggingFlag_,
      processorFactory_,
      processor_,
      serviceMetadata_,
      threadManager_,
      serverConfigs_,
      requestsRegistry_,
      errorHandler_,
      server_,
      worker_);

  orchestrator.handleRequestCommon(
      std::move(payload),
      std::forward<F>(makeRequest),
      expectedKind,
      icontext,
      std::bind_front(&RocketRequestHandler::shouldSample, this),
      injectedFault);
}

void RocketRequestHandler::terminateInteraction(int64_t id) {
  if (processor_) {
    processor_->terminateInteraction(id, *connContext_, *eventBase_);
  }
}

void RocketRequestHandler::destroyAllInteractions() {
  if (processor_) {
    processor_->destroyAllInteractions(*connContext_, *eventBase_);
  }
}

void RocketRequestHandler::requestComplete() {
  serverConfigs_->decActiveRequests();
}

void RocketRequestHandler::coalesceProcessorWithLegacyEventHandlers() {
  if (processor_) {
    processor_->coalesceWithServerScopedLegacyEventHandlers(*server_);
  }
}

} // namespace apache::thrift::rocket
