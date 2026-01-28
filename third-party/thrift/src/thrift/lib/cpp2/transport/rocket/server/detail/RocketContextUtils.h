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
#include <cstdint>
#include <memory>
#include <optional>

#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>

#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace folly {
class RequestContext;
class SocketFds;
template <typename T>
class Try;
} // namespace folly

namespace apache::thrift {

class Cpp2RequestContext;
class RequestRpcMetadata;
class RequestsRegistry;

namespace server {
class TServerObserver;
}

namespace rocket {

struct RequestContextData {
  std::optional<int64_t> interactionIdOpt{};
  std::optional<InteractionCreate> interactionCreateOpt{};
  std::optional<uint32_t> crc32Opt{};
  std::optional<CompressionAlgorithm> compressionOpt{};
  std::unique_ptr<folly::IOBuf> frameworkMetadataPtr{};
};

namespace context_utils {

Cpp2Worker::PerServiceMetadata::FindMethodResult findMethodMetadata(
    Cpp2Worker::PerServiceMetadata* serviceMetadata,
    const RequestRpcMetadata& metadata);

std::shared_ptr<folly::RequestContext> createRequestContext(
    Cpp2Worker::PerServiceMetadata* serviceMetadata,
    RequestsRegistry* requestsRegistry,
    const Cpp2Worker::PerServiceMetadata::FindMethodResult&
        methodMetadataResult);

RequestContextData extractRequestContextData(
    const RequestRpcMetadata& metadata);

void setupRequestContext(
    Cpp2RequestContext* cpp2ReqCtx,
    std::chrono::steady_clock::time_point readEnd,
    folly::Try<folly::SocketFds>& tryFds,
    const server::TServerObserver::SamplingStatus& samplingStatus);

void setupObserverCallbacks(
    Cpp2RequestContext* cpp2ReqCtx,
    const server::TServerObserver::SamplingStatus& samplingStatus,
    server::TServerObserver* serverObserver,
    concurrency::ThreadManager* threadManager,
    server::ServerConfigs* serverConfigs);

void setupInteractionContext(
    Cpp2RequestContext* cpp2ReqCtx, const RequestContextData& contextData);

void setupRequestMetadata(
    Cpp2RequestContext* cpp2ReqCtx,
    RpcKind expectedKind,
    size_t wiredPayloadSize,
    const RequestContextData& contextData);

} // namespace context_utils

} // namespace rocket
} // namespace apache::thrift
