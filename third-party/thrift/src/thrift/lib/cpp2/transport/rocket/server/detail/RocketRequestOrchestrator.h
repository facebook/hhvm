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
#include <memory>

#include <fmt/core.h>

#include <folly/io/async/AsyncTransport.h>
#include <folly/synchronization/CallOnce.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEventHelper.h>
#include <thrift/lib/cpp2/server/PreprocessParams.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/server/InteractionOverload.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketContextUtils.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketErrorHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketRequestProcessor.h>
#include <thrift/lib/cpp2/util/Checksum.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DECLARE_bool(enable_interaction_overload_protection_server);

namespace apache::thrift {

class AsyncProcessor;
class AsyncProcessorFactory;
class Cpp2ConnContext;
class RequestsRegistry;

namespace concurrency {
class ThreadManager;
}

namespace server {
class ServerConfigs;
class TServerObserver;
} // namespace server

namespace rocket {

class IRocketServerConnection;
class Payload;
class RocketServerConnection;

struct ParsedPayloadData {
  RequestRpcMetadata metadata;
  std::unique_ptr<folly::IOBuf> data;
  folly::Try<folly::SocketFds> fds;
  ChecksumAlgorithm checksumAlgorithm;
};

struct ExtractedPayloadData {
  RequestRpcMetadata metadata;
  std::unique_ptr<folly::IOBuf> data;
  folly::Try<folly::SocketFds> tryFds;
  ChecksumAlgorithm checksumAlgorithm;
};

template <class RequestType>
struct RequestContextInfo {
  RequestType request;
  Cpp2Worker::PerServiceMetadata::FindMethodResult methodMetadataResult;
  RequestContextData contextData;
  std::shared_ptr<folly::RequestContext> reqCtx;
};

/**
 * RocketRequestOrchestrator handles the complete request processing pipeline
 * for Rocket server connections. It orchestrates payload parsing, validation,
 * context setup, overload checking, and request dispatching.
 */
class RocketRequestOrchestrator {
 public:
  RocketRequestOrchestrator(
      folly::AsyncTransport* transport,
      Cpp2ConnContext& connContext,
      folly::once_flag& setupLoggingFlag,
      AsyncProcessorFactory* processorFactory,
      std::shared_ptr<AsyncProcessor> processor,
      Cpp2Worker::PerServiceMetadata* serviceMetadata,
      std::shared_ptr<concurrency::ThreadManager> threadManager,
      server::ServerConfigs* serverConfigs,
      RequestsRegistry* requestsRegistry,
      RocketErrorHandler* errorHandler,
      ThriftServer* server,
      Cpp2Worker* worker);

  template <class F, class ShouldSampleFunc>
  void handleRequestCommon(
      Payload&& payload,
      F&& makeRequest,
      RpcKind expectedKind,
      IRocketServerConnection& icontext,
      ShouldSampleFunc&& shouldSample,
      std::optional<RocketErrorHandler::InjectedFault> injectedFault);

 private:
  template <class F>
  std::optional<ExtractedPayloadData> parsePayloadAndExtractFds(
      Payload&& payload,
      F&& makeRequest,
      RocketServerConnection& connection,
      rocket::Payload& debugPayload);

  template <class F>
  std::optional<ParsedPayloadData> validateAndProcessPayload(
      ExtractedPayloadData&& extractedData,
      F&& makeRequest,
      RpcKind& expectedKind,
      RocketServerConnection& connection,
      rocket::Payload& debugPayload);

  template <class F>
  std::optional<ParsedPayloadData> parseAndValidatePayload(
      Payload&& payload,
      F&& makeRequest,
      RpcKind& expectedKind,
      RocketServerConnection& connection,
      rocket::Payload& debugPayload);

  template <class F>
  auto createRequestWithContext(
      const ParsedPayloadData& parsedData,
      F&& makeRequest,
      rocket::Payload&& debugPayload)
      -> std::optional<RequestContextInfo<typename std::invoke_result_t<
          F,
          RequestRpcMetadata&&,
          rocket::Payload&&,
          std::shared_ptr<folly::RequestContext>&&>>>;

  template <class RequestType>
  bool validateRequestExecution(
      RequestType& request, const RequestContextData& contextData);

  bool shouldRejectForInteractionOverload(
      const std::optional<int64_t>& interactionIdOpt) const;

  template <class RequestType, class ContextInfo, class ShouldSampleFunc>
  void finalizeAndDispatchRequest(
      RequestType&& request,
      ParsedPayloadData parsedData,
      const ContextInfo& contextInfo,
      RocketServerConnection& connection,
      ShouldSampleFunc&& shouldSample,
      RpcKind expectedKind,
      size_t wiredPayloadSize,
      std::chrono::steady_clock::time_point readEnd);

  folly::AsyncTransport* transport_;
  Cpp2ConnContext& connContext_;
  folly::once_flag& setupLoggingFlag_;
  AsyncProcessorFactory* processorFactory_;
  std::shared_ptr<AsyncProcessor> processor_;
  Cpp2Worker::PerServiceMetadata* serviceMetadata_;
  std::shared_ptr<concurrency::ThreadManager> threadManager_;
  server::ServerConfigs* serverConfigs_;
  RequestsRegistry* requestsRegistry_;
  RocketErrorHandler* errorHandler_;
  ThriftServer* server_;
  Cpp2Worker* worker_;
};

// Template implementations

template <class F>
std::optional<ExtractedPayloadData>
RocketRequestOrchestrator::parsePayloadAndExtractFds(
    Payload&& payload,
    F&& makeRequest,
    RocketServerConnection& connection,
    rocket::Payload& debugPayload) {
  RocketRequestProcessor requestProcessor(transport_);
  auto requestPayloadTry =
      requestProcessor.parseRequestPayload(std::move(payload), connection);

  if (requestPayloadTry.hasException()) {
    serverConfigs_->incActiveRequests();
    errorHandler_->handleDecompressionFailure(
        makeRequest(
            RequestRpcMetadata(),
            rocket::Payload{},
            std::make_shared<folly::RequestContext>(
                requestsRegistry_->genRootId())),
        requestPayloadTry.exception().what().toStdString());
    return std::nullopt;
  }

  auto& metadata = requestPayloadTry.value().metadata;
  auto& data = requestPayloadTry.value().payload;
  auto checksumAlgorithm =
      requestProcessor.setupChecksumHandling(metadata, connection);

  folly::Try<folly::SocketFds> tryFds;
  if (!requestProcessor.extractFileDescriptors(metadata, tryFds)) {
    if (auto* observer = serverConfigs_->getObserver()) {
      observer->taskKilled();
    }
    serverConfigs_->incActiveRequests();
    errorHandler_->handleRequestWithFdsExtractionFailure(
        makeRequest(
            RequestRpcMetadata(metadata),
            std::move(debugPayload),
            std::make_shared<folly::RequestContext>(
                requestsRegistry_->genRootId())),
        tryFds.exception().what().toStdString());
    return std::nullopt;
  }

  return ExtractedPayloadData{
      RequestRpcMetadata(metadata),
      std::move(data),
      std::move(tryFds),
      checksumAlgorithm};
}

template <class F>
std::optional<ParsedPayloadData>
RocketRequestOrchestrator::validateAndProcessPayload(
    ExtractedPayloadData&& extractedData,
    F&& makeRequest,
    RpcKind& expectedKind,
    RocketServerConnection& connection,
    rocket::Payload& debugPayload) {
  RocketRequestProcessor requestProcessor(transport_);

  if (!requestProcessor.validateRequestMetadata(
          extractedData.metadata, expectedKind)) {
    serverConfigs_->incActiveRequests();
    errorHandler_->handleRequestWithBadMetadata(makeRequest(
        RequestRpcMetadata(extractedData.metadata),
        std::move(debugPayload),
        std::make_shared<folly::RequestContext>(
            requestsRegistry_->genRootId())));
    return std::nullopt;
  }

  if (worker_->isStopping()) {
    serverConfigs_->incActiveRequests();
    errorHandler_->handleServerShutdown(makeRequest(
        RequestRpcMetadata(extractedData.metadata),
        std::move(debugPayload),
        std::make_shared<folly::RequestContext>(
            requestsRegistry_->genRootId())));
    return std::nullopt;
  }

  requestProcessor.logApplicationEvents(extractedData.metadata);

  auto errorMsg = requestProcessor.processPayloadCompression(
      extractedData.data, extractedData.metadata, connection);
  if (!errorMsg.empty()) {
    serverConfigs_->incActiveRequests();
    errorHandler_->handleDecompressionFailure(
        makeRequest(
            RequestRpcMetadata(extractedData.metadata),
            std::move(debugPayload),
            std::make_shared<folly::RequestContext>(
                requestsRegistry_->genRootId())),
        std::move(errorMsg));
    return std::nullopt;
  }

  if (!requestProcessor.validateChecksum(
          extractedData.data, extractedData.metadata)) {
    serverConfigs_->incActiveRequests();
    errorHandler_->handleRequestWithBadChecksum(makeRequest(
        RequestRpcMetadata(extractedData.metadata),
        std::move(debugPayload),
        std::make_shared<folly::RequestContext>(
            requestsRegistry_->genRootId())));
    return std::nullopt;
  }

  return ParsedPayloadData{
      std::move(extractedData.metadata),
      std::move(extractedData.data),
      std::move(extractedData.tryFds),
      extractedData.checksumAlgorithm};
}

template <class F>
std::optional<ParsedPayloadData>
RocketRequestOrchestrator::parseAndValidatePayload(
    Payload&& payload,
    F&& makeRequest,
    RpcKind& expectedKind,
    RocketServerConnection& connection,
    rocket::Payload& debugPayload) {
  auto extractedData = parsePayloadAndExtractFds(
      std::move(payload), makeRequest, connection, debugPayload);
  if (!extractedData) {
    return std::nullopt;
  }

  return validateAndProcessPayload(
      std::move(*extractedData),
      std::forward<F>(makeRequest),
      expectedKind,
      connection,
      debugPayload);
}

template <class F>
auto RocketRequestOrchestrator::createRequestWithContext(
    const ParsedPayloadData& parsedData,
    F&& makeRequest,
    rocket::Payload&& debugPayload)
    -> std::optional<RequestContextInfo<typename std::invoke_result_t<
        F,
        RequestRpcMetadata&&,
        rocket::Payload&&,
        std::shared_ptr<folly::RequestContext>&&>>> {
  auto methodMetadataResult =
      context_utils::findMethodMetadata(serviceMetadata_, parsedData.metadata);
  auto reqCtx = context_utils::createRequestContext(
      serviceMetadata_, requestsRegistry_, methodMetadataResult);
  auto contextData =
      context_utils::extractRequestContextData(parsedData.metadata);

  folly::RequestContextScopeGuard rctx(reqCtx);

  if (shouldRejectForInteractionOverload(contextData.interactionIdOpt)) {
    serverConfigs_->incActiveRequests();
    errorHandler_->handleInteractionLoadshedded(makeRequest(
        RequestRpcMetadata(parsedData.metadata),
        std::move(debugPayload),
        std::move(reqCtx)));
    return std::nullopt;
  }

  auto request = makeRequest(
      RequestRpcMetadata(parsedData.metadata),
      std::move(debugPayload),
      std::move(reqCtx));

  using ResultType = RequestContextInfo<typename std::invoke_result_t<
      F,
      RequestRpcMetadata&&,
      rocket::Payload&&,
      std::shared_ptr<folly::RequestContext>&&>>;

  return std::make_optional<ResultType>(ResultType{
      std::move(request),
      std::move(methodMetadataResult),
      std::move(contextData),
      folly::RequestContext::saveContext()});
}

template <class RequestType>
bool RocketRequestOrchestrator::validateRequestExecution(
    RequestType& request, const RequestContextData& contextData) {
  const auto& headers = request->getTHeader().getHeaders();
  const auto& name = request->getMethodName();
  auto overloadResult = serverConfigs_->checkOverload(headers, name);
  serverConfigs_->incActiveRequests();

  if (UNLIKELY(overloadResult.has_value())) {
    if ((contextData.interactionCreateOpt || contextData.interactionIdOpt) &&
        shouldTerminateInteraction(
            contextData.interactionCreateOpt.has_value(),
            contextData.interactionIdOpt,
            &connContext_)) {
      overloadResult->errorCode = mapToTerminalError(overloadResult->errorCode);
    }
    errorHandler_->handleRequestOverloadedServer(
        std::move(request), std::move(*overloadResult));
    return false;
  }

  if (!serverConfigs_->shouldHandleRequestForMethod(name)) {
    errorHandler_->handleServerNotReady(std::move(request));
    return false;
  }

  auto preprocessResult =
      serverConfigs_->preprocess({headers, name, connContext_, request.get()});
  if (UNLIKELY(!std::holds_alternative<std::monostate>(preprocessResult))) {
    auto interactionIdOptCopy = contextData.interactionIdOpt;
    errorHandler_->handlePreprocessError(
        std::move(request),
        std::move(preprocessResult),
        contextData.interactionCreateOpt.has_value(),
        interactionIdOptCopy);
    return false;
  }

  return true;
}

template <class RequestType, class ContextInfo, class ShouldSampleFunc>
void RocketRequestOrchestrator::finalizeAndDispatchRequest(
    RequestType&& request,
    ParsedPayloadData parsedData,
    const ContextInfo& contextInfo,
    RocketServerConnection& connection,
    ShouldSampleFunc&& shouldSample,
    RpcKind expectedKind,
    size_t wiredPayloadSize,
    std::chrono::steady_clock::time_point readEnd) {
  logSetupConnectionEventsOnce(setupLoggingFlag_, connContext_);

  auto* cpp2ReqCtx = request->getRequestContext();
  auto samplingStatus = shouldSample(request->getTHeader());

  folly::RequestContextScopeGuard rctx(contextInfo.reqCtx);

  auto tryFds = std::move(parsedData.fds);
  context_utils::setupRequestContext(
      cpp2ReqCtx, readEnd, tryFds, samplingStatus);

  context_utils::setupObserverCallbacks(
      cpp2ReqCtx,
      samplingStatus,
      serverConfigs_->getObserver(),
      threadManager_.get(),
      serverConfigs_);

  context_utils::setupInteractionContext(cpp2ReqCtx, contextInfo.contextData);

  context_utils::setupRequestMetadata(
      cpp2ReqCtx, expectedKind, wiredPayloadSize, contextInfo.contextData);

  auto serializedCompressedRequest = SerializedCompressedRequest(
      std::move(parsedData.data),
      contextInfo.contextData.crc32Opt
          ? CompressionAlgorithm::NONE
          : contextInfo.contextData.compressionOpt.value_or(
                CompressionAlgorithm::NONE),
      parsedData.checksumAlgorithm,
      connection.getPayloadSerializer());

  const auto protocolId = request->getProtoId();
  Cpp2Worker::dispatchRequest(
      *processorFactory_,
      processor_.get(),
      std::forward<RequestType>(request),
      std::move(serializedCompressedRequest),
      contextInfo.methodMetadataResult,
      protocolId,
      cpp2ReqCtx,
      threadManager_.get(),
      server_);
}

template <class F, class ShouldSampleFunc>
void RocketRequestOrchestrator::handleRequestCommon(
    Payload&& payload,
    F&& makeRequest,
    RpcKind expectedKind,
    IRocketServerConnection& icontext,
    ShouldSampleFunc&& shouldSample,
    std::optional<RocketErrorHandler::InjectedFault> injectedFault) {
  auto& connection = static_cast<RocketServerConnection&>(icontext);
  auto readEnd = std::chrono::steady_clock::now();
  auto wiredPayloadSize = payload.metadataAndDataSize();
  rocket::Payload debugPayload = payload.clone();

  RpcKind actualKind = expectedKind;
  auto parsedData = parseAndValidatePayload(
      std::move(payload), makeRequest, actualKind, connection, debugPayload);
  if (!parsedData) {
    return;
  }

  if (injectedFault.has_value()) {
    serverConfigs_->incActiveRequests();
    errorHandler_->handleInjectedFault(
        makeRequest(
            RequestRpcMetadata(parsedData->metadata),
            std::move(debugPayload),
            std::make_shared<folly::RequestContext>(
                requestsRegistry_->genRootId())),
        *injectedFault);
    return;
  }

  auto contextInfo = createRequestWithContext(
      *parsedData, std::forward<F>(makeRequest), std::move(debugPayload));
  if (!contextInfo) {
    return;
  }

  if (!validateRequestExecution(
          contextInfo->request, contextInfo->contextData)) {
    return;
  }

  finalizeAndDispatchRequest(
      std::move(contextInfo->request),
      std::move(*parsedData),
      *contextInfo,
      connection,
      std::forward<ShouldSampleFunc>(shouldSample),
      actualKind,
      wiredPayloadSize,
      readEnd);
}

inline bool RocketRequestOrchestrator::shouldRejectForInteractionOverload(
    const std::optional<int64_t>& interactionIdOpt) const {
  if (!interactionIdOpt) {
    return false;
  }

  if (!THRIFT_FLAG(enable_interaction_overload_protection_server)) {
    return false;
  }

  auto* interaction =
      apache::thrift::detail::Cpp2ConnContextInternalAPI(connContext_)
          .findTile(*interactionIdOpt);
  if (!interaction) {
    return false;
  }

  auto* overloadPolicy =
      apache::thrift::detail::TileInternalAPI(*interaction).getOverloadPolicy();
  if (!overloadPolicy) {
    return false;
  }

  return !overloadPolicy->allowNewRequest();
}

} // namespace rocket
} // namespace apache::thrift
