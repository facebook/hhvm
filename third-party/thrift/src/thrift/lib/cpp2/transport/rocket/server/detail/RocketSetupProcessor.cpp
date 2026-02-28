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

#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketSetupProcessor.h>

#include <algorithm>
#include <cstdint>

#include <fmt/core.h>
#include <glog/logging.h>
#include <folly/ExceptionString.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/ScopeGuard.h>
#include <folly/io/Cursor.h>
#include <folly/lang/Bits.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressorRegistry.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameInterceptor.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketErrorHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketRequestHandler.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DECLARE_bool(rocket_server_legacy_protocol_key);

namespace apache::thrift::rocket {

namespace {
constexpr int64_t kRocketServerMinVersion = 8;
} // namespace

RocketSetupProcessor::RocketSetupProcessor(
    Cpp2Worker& worker,
    Cpp2ConnContext& connContext,
    const std::vector<std::unique_ptr<SetupFrameHandler>>& handlers,
    const std::vector<std::unique_ptr<SetupFrameInterceptor>>& interceptors,
    int32_t& version,
    folly::AsyncTransport* transport,
    std::chrono::milliseconds maxResponseWriteTime,
    folly::once_flag* setupLoggingFlag,
    RocketErrorHandler* errorHandler)
    : worker_(worker),
      connContext_(connContext),
      setupFrameHandlers_(handlers),
      setupFrameInterceptors_(interceptors),
      version_(version),
      transport_(transport),
      maxResponseWriteTime_(maxResponseWriteTime),
      setupLoggingFlag_(setupLoggingFlag),
      errorHandler_(errorHandler) {}

std::unique_ptr<RocketRequestHandler> RocketSetupProcessor::handleSetupFrame(
    SetupFrame&& frame,
    RocketServerConnection& connection,
    const std::function<void()>& onConnectionAttempted,
    bool* isCustomHandlerOut) {
  if (!validateSetupFrameBasics(frame, connection)) {
    return nullptr;
  }

  std::unique_ptr<RocketRequestHandler> requestHandler;

  try {
    folly::io::Cursor cursor(frame.payload().buffer());
    if (!validateProtocolKey(cursor, frame, connection)) {
      return nullptr;
    }

    RequestSetupMetadata meta;
    if (!deserializeSetupMetadata(meta, cursor, frame, connection)) {
      return nullptr;
    }

    if (!negotiateProtocolVersion(meta, connection)) {
      return nullptr;
    }

    if (!validateMimeTypes(frame, connection)) {
      return nullptr;
    }

    if (!runSetupInterceptors(meta, connection)) {
      return nullptr;
    }

    bool isCustomHandler = false;
    requestHandler = setupProcessor(meta, connection, isCustomHandler);
    if (!requestHandler) {
      return nullptr;
    }

    // Report custom handler status to caller if requested
    if (isCustomHandlerOut) {
      *isCustomHandlerOut = isCustomHandler;
    }

    if (!isCustomHandler) {
      connection.applyQosMarking(meta);
    }

    if (onConnectionAttempted) {
      onConnectionAttempted();
    }

    if (!isCustomHandler) {
      // Only send metadata push for default handler path.
      configureConnectionSettings(meta, connection);
      // For default handlers, coalesce happens here BEFORE
      // ConnectionEstablished.
      requestHandler->coalesceProcessorWithLegacyEventHandlers();
    }

    return requestHandler; // NOLINT(clang-diagnostic-nrvo)
  } catch (const std::exception& e) {
    connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID_SETUP,
            fmt::format(
                "Error deserializing SETUP payload: {}",
                folly::exceptionStr(e).toStdString())));
    return nullptr;
  }
}

bool RocketSetupProcessor::validateSetupFrameBasics(
    const SetupFrame& frame, RocketServerConnection& connection) {
  if (!frame.payload().hasNonemptyMetadata()) {
    connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID_SETUP,
            "Missing required metadata in SETUP frame"));
    return false;
  }
  return true;
}

bool RocketSetupProcessor::validateProtocolKey(
    folly::io::Cursor& cursor,
    const SetupFrame& frame,
    RocketServerConnection& connection) {
  uint32_t protocolKey;
  const bool success = cursor.tryReadBE<uint32_t>(protocolKey);
  constexpr uint32_t kLegacyRocketProtocolKey = 1;

  if (!success ||
      ((!THRIFT_FLAG(rocket_server_legacy_protocol_key) ||
        protocolKey != kLegacyRocketProtocolKey) &&
       protocolKey != RpcMetadata_constants::kRocketProtocolKey())) {
    if (!frame.rocketMimeTypes()) {
      connection.close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID_SETUP, "Incompatible Thrift version"));
      return false;
    }
    if (success) {
      cursor.retreat(4);
    }
  }
  return true;
}

bool RocketSetupProcessor::deserializeSetupMetadata(
    RequestSetupMetadata& meta,
    folly::io::Cursor& cursor,
    const SetupFrame& frame,
    RocketServerConnection& connection) {
  if (PayloadSerializer::getInstance()->unpack(
          meta, cursor, frame.encodeMetadataUsingBinary()) !=
      frame.payload().metadataSize()) {
    connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID_SETUP,
            "Error deserializing SETUP payload: underflow"));
    return false;
  }
  connContext_.readSetupMetadata(meta);
  return true;
}

bool RocketSetupProcessor::negotiateProtocolVersion(
    const RequestSetupMetadata& meta, RocketServerConnection& connection) {
  auto minVersion = meta.minVersion().value_or(0);
  auto maxVersion = meta.maxVersion().value_or(0);

  THRIFT_CONNECTION_EVENT(rocket.setup).log(connContext_, [&] {
    return folly::dynamic::object("client_min_version", minVersion)(
        "client_max_version", maxVersion)("server_version", version_);
  });

  if (minVersion > version_) {
    connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID_SETUP, "Incompatible Rocket version"));
    return false;
  }

  if (maxVersion < kRocketServerMinVersion) {
    connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID_SETUP, "Incompatible Rocket version"));
    return false;
  }

  version_ = std::min(version_, maxVersion);
  return true;
}

bool RocketSetupProcessor::validateMimeTypes(
    const SetupFrame& frame, RocketServerConnection& connection) {
  if (version_ >= 9 && !frame.rocketMimeTypes()) {
    connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID_SETUP, "Unsupported MIME types"));
    return false;
  }
  return true;
}

bool RocketSetupProcessor::runSetupInterceptors(
    const RequestSetupMetadata& meta, RocketServerConnection& connection) {
  for (const auto& i : setupFrameInterceptors_) {
    auto frameAccepted = i->acceptSetup(meta, connContext_);
    if (frameAccepted.hasError()) {
      connection.close(
          folly::make_exception_wrapper<RocketException>(
              ErrorCode::INVALID_SETUP, frameAccepted.error().what()));
      return false;
    }
  }
  return true;
}

std::unique_ptr<RocketRequestHandler> RocketSetupProcessor::setupProcessor(
    const RequestSetupMetadata& meta,
    RocketServerConnection& connection,
    bool& isCustomHandler) {
  for (const auto& h : setupFrameHandlers_) {
    auto processorInfo = h->tryHandle(meta);
    if (processorInfo) {
      auto sharedProcessorInfo =
          std::make_shared<ProcessorInfo>(std::move(*processorInfo));
      isCustomHandler = true;
      return setupCustomProcessor(sharedProcessorInfo, connection);
    }
  }

  isCustomHandler = false;
  return setupDefaultProcessor();
}

std::unique_ptr<RocketRequestHandler>
RocketSetupProcessor::setupCustomProcessor(
    const std::shared_ptr<ProcessorInfo>& processorInfo,
    RocketServerConnection& connection) {
  AsyncProcessorFactory* processorFactory =
      std::addressof(processorInfo->processorFactory_);
  Cpp2Worker::PerServiceMetadata* serviceMetadata =
      std::addressof(worker_.getMetadataForService(*processorFactory));
  auto processor = processorFactory->getProcessor();
  auto threadManager = std::move(processorInfo->threadManager_);

  RequestsRegistry* requestsRegistry =
      processorInfo->requestsRegistry_ != nullptr
      ? processorInfo->requestsRegistry_
      : worker_.getRequestsRegistry();

  bool valid = true;
  valid &= !!processor;
  valid &=
      (!!threadManager || processorInfo->useResourcePool_ ||
       !worker_.getServer()->resourcePoolSet().empty());

  if (!valid) {
    connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::INVALID_SETUP,
            "Error in implementation of custom connection handler."));
    return nullptr;
  }

  server::ServerConfigs* serverConfigs = worker_.getServer();
  folly::EventBase* eventBase = connContext_.getTransport()->getEventBase();
  uint32_t sampleRate = 0;

  return std::make_unique<RocketRequestHandler>(
      processorFactory,
      nullptr, // processorFactoryStorage (not used for custom)
      serviceMetadata,
      std::move(processor),
      std::move(threadManager),
      serverConfigs,
      requestsRegistry,
      eventBase,
      &connContext_,
      worker_.getServer(),
      &worker_,
      transport_,
      sampleRate,
      version_,
      maxResponseWriteTime_,
      setupLoggingFlag_,
      errorHandler_);
}

std::unique_ptr<RocketRequestHandler>
RocketSetupProcessor::setupDefaultProcessor() {
  AsyncProcessorFactory* processorFactory =
      std::addressof(worker_.getServer()->getDecoratedProcessorFactory());
  std::shared_ptr<AsyncProcessorFactory> processorFactoryStorage;

  if (auto newConnectionContext = ThriftServer::extractNewConnectionContext(
          const_cast<folly::AsyncTransport&>(*connContext_.getTransport()))) {
    processorFactoryStorage = newConnectionContext->processorFactory;
    processorFactory = std::addressof(*processorFactoryStorage);
  }

  Cpp2Worker::PerServiceMetadata* serviceMetadata =
      std::addressof(worker_.getMetadataForService(
          *processorFactory, processorFactoryStorage));
  auto processor = processorFactory->getProcessor();
  std::shared_ptr<concurrency::ThreadManager> threadManager;

  if (worker_.getServer()->resourcePoolSet().empty()) {
    threadManager = worker_.getServer()->getThreadManager_deprecated();
  }

  RequestsRegistry* requestsRegistry = worker_.getRequestsRegistry();
  server::ServerConfigs* serverConfigs = worker_.getServer();
  folly::EventBase* eventBase = connContext_.getTransport()->getEventBase();

  uint32_t sampleRate = 0;
  if (auto* observer = serverConfigs->getObserver()) {
    sampleRate = observer->getSampleRate();
  }

  return std::make_unique<RocketRequestHandler>(
      processorFactory,
      std::move(processorFactoryStorage),
      serviceMetadata,
      std::move(processor),
      std::move(threadManager),
      serverConfigs,
      requestsRegistry,
      eventBase,
      &connContext_,
      worker_.getServer(),
      &worker_,
      transport_,
      sampleRate,
      version_,
      maxResponseWriteTime_,
      setupLoggingFlag_,
      errorHandler_);
}

void RocketSetupProcessor::configureConnectionSettings(
    const RequestSetupMetadata& meta, RocketServerConnection& connection) {
  ServerPushMetadata serverMeta;
  serverMeta.set_setupResponse();
  serverMeta.setupResponse()->version() = version_;
  serverMeta.setupResponse()->zstdSupported() = true;

  if (auto ref = meta.compressionSetupRequest()) {
    auto compressionSetupRes =
        handleSetupFrameCustomCompression(*ref, connection);
    if (compressionSetupRes.hasError()) {
      LOG(WARNING) << fmt::format(
          "Error setting up custom compression: {}, fallback to not using custom compression.",
          compressionSetupRes.error());
    } else {
      auto optResponse = std::move(compressionSetupRes.value());
      if (optResponse) {
        serverMeta.setupResponse()
            ->compressionSetupResponse()
            .ensure()
            .custom() = std::move(*optResponse);
      }
      // otherwise, custom compression is simply not used
    }
  }

  connection.sendMetadataPush(
      PayloadSerializer::getInstance()->packCompact(serverMeta));
}

folly::Expected<std::optional<CustomCompressionSetupResponse>, std::string>
RocketSetupProcessor::handleSetupFrameCustomCompression(
    const CompressionSetupRequest& setupRequest,
    RocketServerConnection& connection) {
  if (!setupRequest.custom()) {
    return folly::makeUnexpected(
        "Cannot setup compression on server due to unrecognized request type");
  }
  const auto& customSetupRequest = *setupRequest.custom();

  auto factory =
      CustomCompressorRegistry::get(*customSetupRequest.compressorName());
  if (!factory) {
    return folly::makeUnexpected(
        fmt::format(
            "Custom compressor {} is not supported on server.",
            *customSetupRequest.compressorName()));
  }

  std::optional<CustomCompressionSetupResponse> response;
  try {
    response =
        factory->createCustomCompressorNegotiationResponse(customSetupRequest);
  } catch (const std::exception& ex) {
    return folly::makeUnexpected(
        fmt::format(
            "Failed to create negotiation response on server due to: {}",
            ex.what()));
  }

  if (!response) {
    // custom compression chooses not to be used for this connection
    return folly::makeExpected<std::string>(response);
  }

  std::shared_ptr<CustomCompressor> compressor;
  try {
    compressor = factory->make(
        customSetupRequest,
        *response,
        CustomCompressorFactory::CompressorLocation::SERVER);
  } catch (const std::exception& ex) {
    return folly::makeUnexpected(
        fmt::format(
            "Failed to create custom compressor on server due to: {}",
            ex.what()));
  }

  if (!compressor) {
    return folly::makeUnexpected(
        fmt::format(
            "Failed to create custom compressor on server due to unknown error."));
  }

  connection.applyCustomCompression(compressor);
  return folly::makeExpected<std::string>(std::move(*response));
}

} // namespace apache::thrift::rocket
