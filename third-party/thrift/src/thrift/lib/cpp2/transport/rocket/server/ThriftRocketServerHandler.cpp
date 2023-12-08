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

#include <thrift/lib/cpp2/transport/rocket/server/ThriftRocketServerHandler.h>

#include <memory>
#include <utility>

#include <fmt/core.h>
#include <folly/ExceptionString.h>
#include <folly/ExceptionWrapper.h>
#include <folly/GLog.h>
#include <folly/Overload.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEventHelper.h>
#include <thrift/lib/cpp2/server/MonitoringMethodNames.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/PayloadUtils.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketThriftRequests.h>
#include <thrift/lib/cpp2/util/Checksum.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>

namespace {
const int64_t kRocketServerMaxVersion = 10;
const int64_t kRocketServerMinVersion = 8;
} // namespace

THRIFT_FLAG_DEFINE_bool(rocket_server_legacy_protocol_key, true);
THRIFT_FLAG_DEFINE_int64(rocket_server_max_version, kRocketServerMaxVersion);

namespace apache {
namespace thrift {
namespace rocket {

thread_local uint32_t ThriftRocketServerHandler::sample_{0};

namespace {
bool isMetadataValid(const RequestRpcMetadata& metadata, RpcKind expectedKind) {
  return metadata.protocol_ref() && metadata.name_ref() &&
      metadata.kind_ref() && metadata.kind_ref() == expectedKind;
}
} // namespace

ThriftRocketServerHandler::ThriftRocketServerHandler(
    std::shared_ptr<Cpp2Worker> worker,
    const folly::SocketAddress& clientAddress,
    folly::AsyncTransport* transport,
    const std::vector<std::unique_ptr<SetupFrameHandler>>& handlers)
    : worker_(std::move(worker)),
      connectionGuard_(worker_->getActiveRequestsGuard()),
      transport_(transport),
      connContext_(
          &clientAddress,
          transport,
          nullptr, /* eventBaseManager */
          nullptr, /* x509PeerCert */
          worker_->getServer()->getClientIdentityHook(),
          worker_.get()),
      setupFrameHandlers_(handlers),
      version_(static_cast<int32_t>(std::min(
          kRocketServerMaxVersion, THRIFT_FLAG(rocket_server_max_version)))),
      maxResponseWriteTime_(worker_->getServer()
                                ->getThriftServerConfig()
                                .getMaxResponseWriteTime()
                                .get()) {
  connContext_.setTransportType(Cpp2ConnContext::TransportType::ROCKET);
  for (const auto& handler : worker_->getServer()->getEventHandlersUnsafe()) {
    handler->newConnection(&connContext_);
  }
}

ThriftRocketServerHandler::~ThriftRocketServerHandler() {
  for (const auto& handler : worker_->getServer()->getEventHandlersUnsafe()) {
    handler->connectionDestroyed(&connContext_);
  }
  // Ensure each connAccepted() call has a matching connClosed()
  if (auto* observer = worker_->getServer()->getObserver()) {
    observer->connClosed();
  }
}

apache::thrift::server::TServerObserver::SamplingStatus
ThriftRocketServerHandler::shouldSample(const transport::THeader& header) {
  bool isServerSamplingEnabled =
      (sampleRate_ > 0) && ((sample_++ % sampleRate_) == 0);

  int64_t logSampleRatio = 0;
  int64_t logErrorSampleRatio = 0;
  if (const auto& loggingContext = header.loggingContext()) {
    logSampleRatio = *loggingContext->logSampleRatio();
    logErrorSampleRatio = *loggingContext->logErrorSampleRatio();
  }
  return apache::thrift::server::TServerObserver::SamplingStatus(
      isServerSamplingEnabled, logSampleRatio, logErrorSampleRatio);
}

void ThriftRocketServerHandler::handleSetupFrame(
    SetupFrame&& frame, RocketServerConnection& connection) {
  if (!frame.payload().hasNonemptyMetadata()) {
    return connection.close(folly::make_exception_wrapper<RocketException>(
        ErrorCode::INVALID_SETUP, "Missing required metadata in SETUP frame"));
  }

  folly::io::Cursor cursor(frame.payload().buffer());

  // Validate Thrift protocol key
  uint32_t protocolKey;
  const bool success = cursor.tryReadBE<uint32_t>(protocolKey);
  constexpr uint32_t kLegacyRocketProtocolKey = 1;
  if (!success ||
      ((!THRIFT_FLAG(rocket_server_legacy_protocol_key) ||
        protocolKey != kLegacyRocketProtocolKey) &&
       protocolKey != RpcMetadata_constants::kRocketProtocolKey())) {
    if (!frame.rocketMimeTypes()) {
      return connection.close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID_SETUP, "Incompatible Thrift version"));
    }
    // If Rocket MIME types are used the protocol key is optional.
    if (success) {
      cursor.retreat(4);
    }
  }

  try {
    CompactProtocolReader reader;
    reader.setInput(cursor);
    RequestSetupMetadata meta;
    // Throws on read error
    meta.read(&reader);
    if (reader.getCursorPosition() > frame.payload().metadataSize()) {
      return connection.close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID_SETUP,
          "Error deserializing SETUP payload: underflow"));
    }
    connContext_.readSetupMetadata(meta);

    auto minVersion = meta.minVersion_ref().value_or(0);
    auto maxVersion = meta.maxVersion_ref().value_or(0);

    THRIFT_CONNECTION_EVENT(rocket.setup).log(connContext_, [&] {
      return folly::dynamic::object("client_min_version", minVersion)(
          "client_max_version", maxVersion)("server_version", version_);
    });

    if (minVersion > version_) {
      return connection.close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID_SETUP, "Incompatible Rocket version"));
    }

    if (maxVersion < kRocketServerMinVersion) {
      return connection.close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID_SETUP, "Incompatible Rocket version"));
    }
    version_ = std::min(version_, maxVersion);

    if (version_ >= 9 && !frame.rocketMimeTypes()) {
      return connection.close(folly::make_exception_wrapper<RocketException>(
          ErrorCode::INVALID_SETUP, "Unsupported MIME types"));
    }

    eventBase_ = connContext_.getTransport()->getEventBase();
    for (const auto& h : setupFrameHandlers_) {
      auto processorInfo = h->tryHandle(meta);
      if (processorInfo) {
        bool valid = true;
        processorFactory_ = std::addressof(processorInfo->processorFactory_);
        serviceMetadata_ =
            std::addressof(worker_->getMetadataForService(*processorFactory_));
        valid &= !!(processor_ = processorFactory_->getProcessor());
        // Allow no thread manager if resource pools in use
        valid &=
            (!!(threadManager_ = std::move(processorInfo->threadManager_)) ||
             processorInfo->useResourcePool_ ||
             !worker_->getServer()->resourcePoolSet().empty());
        valid &= !!(serverConfigs_ = &processorInfo->serverConfigs_);
        requestsRegistry_ = processorInfo->requestsRegistry_ != nullptr
            ? processorInfo->requestsRegistry_
            : worker_->getRequestsRegistry();
        if (!valid) {
          return connection.close(
              folly::make_exception_wrapper<RocketException>(
                  ErrorCode::INVALID_SETUP,
                  "Error in implementation of custom connection handler."));
        }
        return;
      }
    }
    // no custom frame handler was found, do the default
    processorFactory_ =
        std::addressof(worker_->getServer()->getDecoratedProcessorFactory());
    if (auto newConnectionContext = ThriftServer::extractNewConnectionContext(
            const_cast<folly::AsyncTransport&>(*connContext_.getTransport()))) {
      processorFactoryStorage_ = newConnectionContext->processorFactory;
      processorFactory_ = std::addressof(*processorFactoryStorage_);
    }
    serviceMetadata_ = std::addressof(worker_->getMetadataForService(
        *processorFactory_, processorFactoryStorage_));
    processor_ = processorFactory_->getProcessor();
    if (worker_->getServer()->resourcePoolSet().empty()) {
      threadManager_ = worker_->getServer()->getThreadManager_deprecated();
    }
    serverConfigs_ = worker_->getServer();
    requestsRegistry_ = worker_->getRequestsRegistry();

    if (auto* observer = serverConfigs_->getObserver()) {
      sampleRate_ = observer->getSampleRate();
    }

    connection.applyQosMarking(meta);

    ServerPushMetadata serverMeta;
    serverMeta.set_setupResponse();
    serverMeta.setupResponse_ref()->version_ref() = version_;
    serverMeta.setupResponse_ref()->zstdSupported_ref() = true;
    CompactProtocolWriter compactProtocolWriter;
    folly::IOBufQueue queue;
    compactProtocolWriter.setOutput(&queue);
    serverMeta.write(&compactProtocolWriter);
    connection.sendMetadataPush(std::move(queue).move());

  } catch (const std::exception& e) {
    return connection.close(folly::make_exception_wrapper<RocketException>(
        ErrorCode::INVALID_SETUP,
        fmt::format(
            "Error deserializing SETUP payload: {}",
            folly::exceptionStr(e).toStdString())));
  }
}

void ThriftRocketServerHandler::handleRequestResponseFrame(
    RequestResponseFrame&& frame, RocketServerFrameContext&& context) {
  auto makeRequestResponse = [&](RequestRpcMetadata&& md,
                                 rocket::Payload&& debugPayload,
                                 std::shared_ptr<folly::RequestContext> ctx) {
    // Note, we're passing connContext by reference and rely on the next
    // chain of ownership to keep it alive: ThriftServerRequestResponse
    // stores RocketServerFrameContext, which keeps refcount on
    // RocketServerConnection, which in turn keeps ThriftRocketServerHandler
    // alive, which in turn keeps connContext_ alive.
    return RequestsRegistry::makeRequest<ThriftServerRequestResponse>(
        *eventBase_,
        *serverConfigs_,
        std::move(md),
        connContext_,
        std::move(ctx),
        *requestsRegistry_,
        std::move(debugPayload),
        std::move(context),
        version_,
        maxResponseWriteTime_);
  };

  handleRequestCommon(
      std::move(frame.payload()),
      std::move(makeRequestResponse),
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
}

void ThriftRocketServerHandler::handleRequestFnfFrame(
    RequestFnfFrame&& frame, RocketServerFrameContext&& context) {
  auto makeRequestFnf = [&](RequestRpcMetadata&& md,
                            rocket::Payload&& debugPayload,
                            std::shared_ptr<folly::RequestContext> ctx) {
    // Note, we're passing connContext by reference and rely on a complex
    // chain of ownership (see handleRequestResponseFrame for detailed
    // explanation).
    return RequestsRegistry::makeRequest<ThriftServerRequestFnf>(
        *eventBase_,
        *serverConfigs_,
        std::move(md),
        connContext_,
        std::move(ctx),
        *requestsRegistry_,
        std::move(debugPayload),
        std::move(context),
        [keepAlive = processor_] {});
  };

  handleRequestCommon(
      std::move(frame.payload()),
      std::move(makeRequestFnf),
      RpcKind::SINGLE_REQUEST_NO_RESPONSE);
}

void ThriftRocketServerHandler::handleRequestStreamFrame(
    RequestStreamFrame&& frame,
    RocketServerFrameContext&& context,
    RocketStreamClientCallback* clientCallback) {
  auto makeRequestStream = [&](RequestRpcMetadata&& md,
                               rocket::Payload&& debugPayload,
                               std::shared_ptr<folly::RequestContext> ctx) {
    return RequestsRegistry::makeRequest<ThriftServerRequestStream>(
        *eventBase_,
        *serverConfigs_,
        std::move(md),
        connContext_,
        std::move(ctx),
        *requestsRegistry_,
        std::move(debugPayload),
        std::move(context),
        version_,
        clientCallback,
        processor_);
  };

  handleRequestCommon(
      std::move(frame.payload()),
      std::move(makeRequestStream),
      RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE);
}

void ThriftRocketServerHandler::handleRequestChannelFrame(
    RequestChannelFrame&& frame,
    RocketServerFrameContext&& context,
    RocketSinkClientCallback* clientCallback) {
  auto makeRequestSink = [&](RequestRpcMetadata&& md,
                             rocket::Payload&& debugPayload,
                             std::shared_ptr<folly::RequestContext> ctx) {
    return RequestsRegistry::makeRequest<ThriftServerRequestSink>(
        *eventBase_,
        *serverConfigs_,
        std::move(md),
        connContext_,
        std::move(ctx),
        *requestsRegistry_,
        std::move(debugPayload),
        std::move(context),
        version_,
        clientCallback,
        processor_);
  };

  handleRequestCommon(
      std::move(frame.payload()), std::move(makeRequestSink), RpcKind::SINK);
}

void ThriftRocketServerHandler::connectionClosing() {
  connContext_.connectionClosed();
  if (processor_) {
    processor_->destroyAllInteractions(connContext_, *eventBase_);
  }
}

template <class F>
void ThriftRocketServerHandler::handleRequestCommon(
    Payload&& payload, F&& makeRequest, RpcKind expectedKind) {
  std::chrono::steady_clock::time_point readEnd =
      std::chrono::steady_clock::now();

  rocket::Payload debugPayload = payload.clone();
  auto requestPayloadTry =
      unpackAsCompressed<RequestPayload>(std::move(payload));

  auto makeActiveRequest = [&](auto&& md, auto&& payload, auto&& reqCtx) {
    serverConfigs_->incActiveRequests();
    return makeRequest(std::move(md), std::move(payload), std::move(reqCtx));
  };

  auto createDefaultRequestContext = [&] {
    return std::make_shared<folly::RequestContext>(
        requestsRegistry_->genRootId());
  };

  if (requestPayloadTry.hasException()) {
    handleDecompressionFailure(
        makeActiveRequest(
            RequestRpcMetadata(),
            rocket::Payload{},
            createDefaultRequestContext()),
        requestPayloadTry.exception().what().toStdString());
    return;
  }

  auto& data = requestPayloadTry->payload;
  auto& metadata = requestPayloadTry->metadata;

  // Extract FDs as early as possible to avoid holding them open if the
  // request fails.
  //
  // The `transport_` raw pointer is valid for the entire lifetime of this
  // class -- see in `~ThriftRocketServerHandler` and the comment in
  // `handleRequestResponseFrame` for evidence.
  folly::Try<folly::SocketFds> tryFds;
  if (metadata.fdMetadata().has_value()) {
    const auto& fdMetadata = *metadata.fdMetadata();
    tryFds = popReceivedFdsFromSocket(
        transport_,
        fdMetadata.numFds().value_or(0),
        fdMetadata.fdSeqNum().value_or(folly::SocketFds::kNoSeqNum));
    if (tryFds.hasException()) {
      if (auto* observer = serverConfigs_->getObserver()) {
        observer->taskKilled();
      }
      handleRequestWithFdsExtractionFailure(
          makeActiveRequest(
              std::move(metadata),
              std::move(debugPayload),
              createDefaultRequestContext()),
          tryFds.exception().what().toStdString());
      return;
    }
  }

  if (!isMetadataValid(metadata, expectedKind)) {
    handleRequestWithBadMetadata(makeActiveRequest(
        std::move(metadata),
        std::move(debugPayload),
        createDefaultRequestContext()));
    return;
  }

  if (worker_->isStopping()) {
    handleServerShutdown(makeActiveRequest(
        std::move(metadata),
        std::move(debugPayload),
        createDefaultRequestContext()));
    return;
  }

  THRIFT_APPLICATION_EVENT(server_read_headers).log([&] {
    auto size =
        metadata.otherMetadata_ref() ? metadata.otherMetadata_ref()->size() : 0;
    std::vector<folly::dynamic> keys;
    if (size) {
      keys.reserve(size);
      for (auto& [k, v] : *metadata.otherMetadata_ref()) {
        keys.push_back(k);
      }
    }
    int fmd_sz = 0;
    if (auto fmd = metadata.frameworkMetadata_ref()) {
      DCHECK(*fmd) << "initialized IOBuf is null";
      fmd_sz = (**fmd).computeChainDataLength();
    }
    return folly::dynamic::object("size", size) //
        ("keys", folly::dynamic::array(std::move(keys))) //
        ("frameworkMetadataSize", fmd_sz);
  });

  if (metadata.crc32c_ref()) {
    try {
      if (auto compression = metadata.compression_ref()) {
        data = uncompressBuffer(std::move(data), *compression);
      }
    } catch (...) {
      handleDecompressionFailure(
          makeActiveRequest(
              std::move(metadata),
              rocket::Payload{},
              createDefaultRequestContext()),
          folly::exceptionStr(std::current_exception()).toStdString());
      return;
    }
  }

  // check the checksum
  const bool badChecksum = metadata.crc32c_ref() &&
      (*metadata.crc32c_ref() != checksum::crc32c(*data));

  if (badChecksum) {
    handleRequestWithBadChecksum(makeActiveRequest(
        std::move(metadata),
        std::move(debugPayload),
        createDefaultRequestContext()));
    return;
  }

  if (auto injectedFailure = worker_->getServer()->maybeInjectFailure();
      injectedFailure != ThriftServer::InjectedFailure::NONE) {
    InjectedFault injectedFault;
    switch (injectedFailure) {
      case ThriftServer::InjectedFailure::NONE:
        folly::assume_unreachable();
      case ThriftServer::InjectedFailure::ERROR:
        injectedFault = InjectedFault::ERROR;
        break;
      case ThriftServer::InjectedFailure::DROP:
        injectedFault = InjectedFault::DROP;
        break;
      case ThriftServer::InjectedFailure::DISCONNECT:
        injectedFault = InjectedFault::DISCONNECT;
        break;
    }
    handleInjectedFault(
        makeActiveRequest(
            std::move(metadata),
            std::move(debugPayload),
            createDefaultRequestContext()),
        injectedFault);
    return;
  }

  using PerServiceMetadata = Cpp2Worker::PerServiceMetadata;
  const PerServiceMetadata::FindMethodResult methodMetadataResult =
      serviceMetadata_->findMethod(
          metadata.name_ref()
              ? metadata.name_ref()->view()
              : std::string_view{}); // need to call with empty string_view
                                     // because we still distinguish
                                     // between NotImplemented and
                                     // MetadataNotFound

  auto rootid = requestsRegistry_->genRootId();
  auto baseReqCtx =
      serviceMetadata_->getBaseContextForRequest(methodMetadataResult);
  auto reqCtx = baseReqCtx
      ? folly::RequestContext::copyAsRoot(*baseReqCtx, rootid)
      : std::make_shared<folly::RequestContext>(rootid);
  folly::RequestContextScopeGuard rctx(reqCtx);

  // A request should not be active until the overload checking is done.
  auto request = makeRequest(
      std::move(metadata), std::move(debugPayload), std::move(reqCtx));

  // check if server is overloaded
  const auto& headers = request->getTHeader().getHeaders();
  const auto& name = request->getMethodName();

  auto overloadResult = serverConfigs_->checkOverload(&headers, &name);
  serverConfigs_->incActiveRequests();
  if (UNLIKELY(overloadResult.has_value())) {
    auto [errorCode, errorMessage] = overloadResult.value();
    handleRequestOverloadedServer(std::move(request), errorCode, errorMessage);
    return;
  }

  if (!serverConfigs_->shouldHandleRequestForMethod(name)) {
    handleServerNotReady(std::move(request));
    return;
  }

  auto preprocessResult =
      serverConfigs_->preprocess({headers, name, connContext_, request.get()});
  if (UNLIKELY(!std::holds_alternative<std::monostate>(preprocessResult))) {
    folly::variant_match(
        preprocessResult,
        [&](AppClientException& ace) {
          handleAppError(std::move(request), ace);
        },
        [&](const AppServerException& ase) {
          handleAppError(std::move(request), ase);
        },
        [&](AppOverloadedException& aoe) {
          handleRequestOverloadedServer(
              std::move(request), kAppOverloadedErrorCode, aoe.getMessage());
        },
        [&](AppQuotaExceededException& aqe) {
          handleQuotaExceededException(
              std::move(request),
              kTenantQuotaExceededErrorCode,
              aqe.getMessage());
        },
        [](std::monostate&) { folly::assume_unreachable(); });

    return;
  }

  logSetupConnectionEventsOnce(setupLoggingFlag_, connContext_);

  auto* cpp2ReqCtx = request->getRequestContext();
  auto& timestamps = cpp2ReqCtx->getTimestamps();
  auto samplingStatus = shouldSample(request->getTHeader());
  timestamps.setStatus(samplingStatus);
  timestamps.readEnd = readEnd;
  timestamps.processBegin = std::chrono::steady_clock::now();

  if (tryFds.hasValue()) {
    cpp2ReqCtx->getHeader()->fds.dcheckEmpty() =
        std::move(tryFds->dcheckReceivedOrEmpty());
  }

  if (auto* observer = serverConfigs_->getObserver()) {
    observer->admittedRequest(&request->getMethodName());
    // Expensive operations; happens only when sampling is enabled
    if (samplingStatus.isServerSamplingEnabled()) {
      if (threadManager_) {
        observer->queuedRequests(threadManager_->pendingUpstreamTaskCount());
      } else if (!serverConfigs_->resourcePoolSet().empty()) {
        observer->queuedRequests(serverConfigs_->resourcePoolSet().numQueued());
      }
      observer->activeRequests(serverConfigs_->getActiveRequests());
    }
  }
  const auto protocolId = request->getProtoId();
  if (auto interactionId = metadata.interactionId_ref()) {
    cpp2ReqCtx->setInteractionId(*interactionId);
  }
  if (auto interactionCreate = metadata.interactionCreate_ref()) {
    cpp2ReqCtx->setInteractionCreate(*interactionCreate);
    DCHECK_EQ(cpp2ReqCtx->getInteractionId(), 0);
    cpp2ReqCtx->setInteractionId(*interactionCreate->interactionId_ref());
  }

  cpp2ReqCtx->setRpcKind(expectedKind);

  if (auto frameworkMetadata = metadata.frameworkMetadata_ref()) {
    cpp2ReqCtx->setFrameworkMetadata(std::move(**frameworkMetadata));
  }

  auto serializedCompressedRequest = SerializedCompressedRequest(
      std::move(data),
      metadata.crc32c_ref()
          ? CompressionAlgorithm::NONE
          : metadata.compression_ref().value_or(CompressionAlgorithm::NONE));

  Cpp2Worker::dispatchRequest(
      processor_.get(),
      std::move(request),
      std::move(serializedCompressedRequest),
      methodMetadataResult,
      protocolId,
      cpp2ReqCtx,
      threadManager_.get(),
      serverConfigs_);
}

void ThriftRocketServerHandler::handleRequestWithBadMetadata(
    ThriftRequestCoreUniquePtr request) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::UNSUPPORTED_CLIENT_TYPE,
          "Invalid metadata object"),
      kRequestParsingErrorCode);
}

void ThriftRocketServerHandler::handleRequestWithBadChecksum(
    ThriftRequestCoreUniquePtr request) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::CHECKSUM_MISMATCH, "Checksum mismatch"),
      kChecksumMismatchErrorCode);
}

void ThriftRocketServerHandler::handleDecompressionFailure(
    ThriftRequestCoreUniquePtr request, std::string&& reason) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::INVALID_TRANSFORM,
          fmt::format("decompression failure: {}", std::move(reason))),
      kRequestParsingErrorCode);
}

void ThriftRocketServerHandler::handleRequestOverloadedServer(
    ThriftRequestCoreUniquePtr request,
    const std::string& errorCode,
    const std::string& errorMessage) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->serverOverloaded();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::LOADSHEDDING, errorMessage),
      errorCode);
  if (request->includeInRecentRequests()) {
    requestsRegistry_->getRequestCounter().incrementOverloadCount();
  }
}

void ThriftRocketServerHandler::handleQuotaExceededException(
    ThriftRequestCoreUniquePtr request,
    const std::string& errorCode,
    const std::string& errorMessage) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::TENANT_QUOTA_EXCEEDED, errorMessage),
      errorCode);
}

void ThriftRocketServerHandler::handleAppError(
    ThriftRequestCoreUniquePtr request,
    const PreprocessResult& appErrorResult) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }

  folly::variant_match(
      appErrorResult,
      [&](const AppClientException& ace) {
        request->sendErrorWrapped(ace, kAppClientErrorCode);
      },
      [&](const AppServerException& ase) {
        request->sendErrorWrapped(ase, kAppServerErrorCode);
      },
      [&](const auto&) { folly::assume_unreachable(); });
}

void ThriftRocketServerHandler::handleRequestWithFdsExtractionFailure(
    ThriftRequestCoreUniquePtr request, std::string&& errorMessage) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::UNKNOWN, std::move(errorMessage)),
      kRequestParsingErrorCode);
  return;
}

void ThriftRocketServerHandler::handleServerNotReady(
    ThriftRequestCoreUniquePtr request) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::LOADSHEDDING, "server not ready"),
      kQueueOverloadedErrorCode);
}

void ThriftRocketServerHandler::handleServerShutdown(
    ThriftRequestCoreUniquePtr request) {
  if (auto* observer = serverConfigs_->getObserver()) {
    observer->taskKilled();
  }
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::LOADSHEDDING, "server shutting down"),
      kQueueOverloadedErrorCode);
}

void ThriftRocketServerHandler::handleInjectedFault(
    ThriftRequestCoreUniquePtr request, InjectedFault fault) {
  switch (fault) {
    case InjectedFault::ERROR:
      if (auto* observer = serverConfigs_->getObserver()) {
        observer->taskKilled();
      }
      request->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::INJECTED_FAILURE, "injected failure"),
          kInjectedFailureErrorCode);
      return;
    case InjectedFault::DROP:
      VLOG(1) << "ERROR: injected drop: "
              << connContext_.getPeerAddress()->getAddressStr();
      return;
    case InjectedFault::DISCONNECT:
      return request->closeConnection(
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::INJECTED_FAILURE, "injected failure"));
      return;
  }
}

void ThriftRocketServerHandler::requestComplete() {
  serverConfigs_->decActiveRequests();
}

void ThriftRocketServerHandler::terminateInteraction(int64_t id) {
  if (processor_) {
    processor_->terminateInteraction(id, connContext_, *eventBase_);
  }
}

void ThriftRocketServerHandler::onBeforeHandleFrame() {
  worker_->getServer()->touchRequestTimestamp();
}
} // namespace rocket
} // namespace thrift
} // namespace apache
