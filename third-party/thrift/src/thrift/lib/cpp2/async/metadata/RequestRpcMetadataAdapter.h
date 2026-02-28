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

#include <stdint.h>
#include <memory>
#include <optional>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

#if __cplusplus >= 202002L
template <typename T>
concept RequestRpcMetadataAdapter = requires(T t) {
  {
    t.protocolId()
  } -> std::same_as<std::optional<std::reference_wrapper<ProtocolId>>>;
  {
    t.name()
  } -> std::same_as<
      std::optional<std::reference_wrapper<ManagedStringViewField>>>;
  { t.kind() } -> std::same_as<std::optional<std::reference_wrapper<RpcKind>>>;
  {
    t.clientTimeoutMs()
  } -> std::same_as<std::optional<std::reference_wrapper<int32_t>>>;
  {
    t.queueTimeoutMs()
  } -> std::same_as<std::optional<std::reference_wrapper<int32_t>>>;
  {
    t.priority()
  } -> std::same_as<std::optional<std::reference_wrapper<RpcPriority>>>;
  {
    t.otherMetadata()
  }
  -> std::same_as<std::optional<folly::F14NodeMap<std::string, std::string>*>>;
  {
    t.crc32c()
  } -> std::same_as<std::optional<std::reference_wrapper<uint32_t>>>;
  {
    t.loadMetric()
  } -> std::same_as<std::optional<std::reference_wrapper<std::string>>>;
  {
    t.compression()
  }
  -> std::same_as<std::optional<std::reference_wrapper<CompressionAlgorithm>>>;
  {
    t.compressionConfig()
  } -> std::same_as<std::optional<std::reference_wrapper<CompressionConfig>>>;
  {
    t.interactionId()
  } -> std::same_as<std::optional<std::reference_wrapper<int64_t>>>;
  {
    t.interactionCreate()
  } -> std::same_as<std::optional<std::reference_wrapper<InteractionCreate>>>;
  {
    t.clientId()
  } -> std::same_as<std::optional<std::reference_wrapper<std::string>>>;
  {
    t.serviceTraceMeta()
  } -> std::same_as<std::optional<std::reference_wrapper<std::string>>>;
  {
    t.frameworkMetadata()
  } -> std::same_as<
      std::optional<std::reference_wrapper<std::unique_ptr<folly::IOBuf>>>>;
  {
    t.fdMetadata()
  } -> std::same_as<std::optional<std::reference_wrapper<FdMetadata>>>;
  {
    t.loggingContext()
  } -> std::same_as<std::optional<std::reference_wrapper<LoggingContext>>>;
  {
    t.tenantId()
  } -> std::same_as<std::optional<std::reference_wrapper<std::string>>>;
  {
    t.checksum()
  } -> std::same_as<std::optional<std::reference_wrapper<Checksum>>>;
  {
    t.checksumResponse()
  } -> std::same_as<std::optional<std::reference_wrapper<bool>>>;
};
#endif

} // namespace apache::thrift
