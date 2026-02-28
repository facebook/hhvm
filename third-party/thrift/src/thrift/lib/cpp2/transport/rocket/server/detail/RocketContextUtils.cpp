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

#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketContextUtils.h>

#include <chrono>
#include <memory>

#include <folly/io/IOBuf.h>
#include <folly/io/async/Request.h>

#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket::context_utils {

Cpp2Worker::PerServiceMetadata::FindMethodResult findMethodMetadata(
    Cpp2Worker::PerServiceMetadata* serviceMetadata,
    const RequestRpcMetadata& metadata) {
  return serviceMetadata->findMethod(
      metadata.name()
          ? metadata.name()->view()
          : std::string_view{}); // need to call with empty string_view
                                 // because we still distinguish
                                 // between NotImplemented and
                                 // MetadataNotFound
}

std::shared_ptr<folly::RequestContext> createRequestContext(
    Cpp2Worker::PerServiceMetadata* serviceMetadata,
    RequestsRegistry* requestsRegistry,
    const Cpp2Worker::PerServiceMetadata::FindMethodResult&
        methodMetadataResult) {
  auto rootid = requestsRegistry->genRootId();
  auto baseReqCtx =
      serviceMetadata->getBaseContextForRequest(methodMetadataResult);
  return baseReqCtx ? folly::RequestContext::copyAsRoot(*baseReqCtx, rootid)
                    : std::make_shared<folly::RequestContext>(rootid);
}

RequestContextData extractRequestContextData(
    const RequestRpcMetadata& metadata) {
  RequestContextData data;
  data.interactionIdOpt = metadata.interactionId().to_optional();
  data.interactionCreateOpt = metadata.interactionCreate().to_optional();
  data.crc32Opt = metadata.crc32c().to_optional();
  data.compressionOpt = metadata.compression().to_optional();
  data.frameworkMetadataPtr = metadata.frameworkMetadata()
      ? (*metadata.frameworkMetadata())->clone()
      : nullptr;
  return data;
}

void setupRequestContext(
    Cpp2RequestContext* cpp2ReqCtx,
    std::chrono::steady_clock::time_point readEnd,
    folly::Try<folly::SocketFds>& tryFds,
    const server::TServerObserver::SamplingStatus& samplingStatus) {
  auto& timestamps = cpp2ReqCtx->getTimestamps();
  timestamps.setStatus(samplingStatus);
  timestamps.readEnd = readEnd;
  timestamps.processBegin = std::chrono::steady_clock::now();

  if (tryFds.hasValue()) {
    cpp2ReqCtx->getHeader()->fds.dcheckEmpty() =
        std::move(tryFds->dcheckReceivedOrEmpty());
  }
}

void setupObserverCallbacks(
    Cpp2RequestContext* cpp2ReqCtx,
    const server::TServerObserver::SamplingStatus& samplingStatus,
    server::TServerObserver* serverObserver,
    concurrency::ThreadManager* threadManager,
    server::ServerConfigs* serverConfigs) {
  if (serverObserver) {
    serverObserver->admittedRequest(&cpp2ReqCtx->getMethodName());
    // Expensive operations; happens only when sampling is enabled
    if (samplingStatus.isServerSamplingEnabled()) {
      if (threadManager) {
        serverObserver->queuedRequests(
            static_cast<int32_t>(threadManager->pendingUpstreamTaskCount()));
      } else if (!serverConfigs->resourcePoolSet().empty()) {
        serverObserver->queuedRequests(
            static_cast<int32_t>(serverConfigs->resourcePoolSet().numQueued()));
      }
      serverObserver->activeRequests(serverConfigs->getActiveRequests());
    }
  }
}

void setupInteractionContext(
    Cpp2RequestContext* cpp2ReqCtx, const RequestContextData& contextData) {
  if (contextData.interactionIdOpt) {
    cpp2ReqCtx->setInteractionId(*contextData.interactionIdOpt);
  }
  if (contextData.interactionCreateOpt) {
    cpp2ReqCtx->setInteractionCreate(*contextData.interactionCreateOpt);
    DCHECK_EQ(cpp2ReqCtx->getInteractionId(), 0);
    cpp2ReqCtx->setInteractionId(
        *contextData.interactionCreateOpt->interactionId());
  }
}

void setupRequestMetadata(
    Cpp2RequestContext* cpp2ReqCtx,
    RpcKind expectedKind,
    size_t wiredPayloadSize,
    const RequestContextData& contextData) {
  cpp2ReqCtx->setRpcKind(expectedKind);

  if (contextData.frameworkMetadataPtr) {
    cpp2ReqCtx->setFrameworkMetadata(
        std::move(*contextData.frameworkMetadataPtr));
  }

  cpp2ReqCtx->setWiredRequestBytes(wiredPayloadSize);
}

} // namespace apache::thrift::rocket::context_utils
