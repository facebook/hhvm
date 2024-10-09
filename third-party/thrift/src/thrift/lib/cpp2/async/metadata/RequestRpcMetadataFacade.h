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

#include <variant>
#include <folly/Overload.h>
#include <thrift/lib/cpp2/async/metadata/CursorBasedRequestRpcMetadataAdapter.h>
#include <thrift/lib/cpp2/async/metadata/RequestRpcMetadataAdapter.h>
#include <thrift/lib/cpp2/async/metadata/TCompactRequestRpcMetadataAdapter.h>

namespace apache::thrift {

class RequestRpcMetadataFacade {
 public:
  explicit RequestRpcMetadataFacade(folly::IOBuf& serializedMetadata);
  explicit RequestRpcMetadataFacade(
      std::unique_ptr<folly::IOBuf>&& serializedMetadata);

  FOLLY_ERASE std::optional<std::reference_wrapper<ProtocolId>> protocolId() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .protocolId();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).protocolId();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<ManagedStringViewField>>
  name() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_).name();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).name();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<RpcKind>> kind() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_).kind();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).kind();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> clientTimeoutMs() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .clientTimeoutMs();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .clientTimeoutMs();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> queueTimeoutMs() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .queueTimeoutMs();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .queueTimeoutMs();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<RpcPriority>> priority() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .priority();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).priority();
    }
  }
  FOLLY_ERASE std::optional<folly::F14NodeMap<std::string, std::string>*>
  otherMetadata() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .otherMetadata();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .otherMetadata();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<uint32_t>> crc32c() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_).crc32c();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).crc32c();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> loadMetric() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .loadMetric();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).loadMetric();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionAlgorithm>>
  compression() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .compression();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .compression();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionConfig>>
  compressionConfig() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .compressionConfig();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .compressionConfig();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<int64_t>> interactionId() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .interactionId();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .interactionId();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<InteractionCreate>>
  interactionCreate() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .interactionCreate();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .interactionCreate();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> clientId() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .clientId();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).clientId();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>>
  serviceTraceMeta() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .serviceTraceMeta();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .serviceTraceMeta();
    }
  }
  FOLLY_ERASE
  std::optional<std::reference_wrapper<std::unique_ptr<folly::IOBuf>>>
  frameworkMetadata() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .frameworkMetadata();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .frameworkMetadata();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<FdMetadata>> fdMetadata() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .fdMetadata();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).fdMetadata();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<LoggingContext>>
  loggingContext() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .loggingContext();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .loggingContext();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> tenantId() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .tenantId();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).tenantId();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<Checksum>> checksum() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .checksum();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_).checksum();
    }
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<bool>> checksumResponse() {
    if (std::holds_alternative<CursorBasedRequestRpcMetadataAdapter>(
            adapter_)) {
      return std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_)
          .checksumResponse();
    } else {
      return std::get<TCompactRequestRpcMetadataAdapter>(adapter_)
          .checksumResponse();
    }
  }

 private:
  std::variant<
      CursorBasedRequestRpcMetadataAdapter,
      TCompactRequestRpcMetadataAdapter>
      adapter_;
};

FOLLY_ERASE RequestRpcMetadataFacade::RequestRpcMetadataFacade(
    std::unique_ptr<folly::IOBuf>&& serializedMetadata)
    : adapter_(std::variant<
               CursorBasedRequestRpcMetadataAdapter,
               TCompactRequestRpcMetadataAdapter>(
          std::in_place_type<TCompactRequestRpcMetadataAdapter>,
          std::move(serializedMetadata))) {}

FOLLY_ERASE RequestRpcMetadataFacade::RequestRpcMetadataFacade(
    folly::IOBuf& serializedMetadata)
    : adapter_(std::variant<
               CursorBasedRequestRpcMetadataAdapter,
               TCompactRequestRpcMetadataAdapter>(
          std::in_place_type<TCompactRequestRpcMetadataAdapter>,
          serializedMetadata)) {}

} // namespace apache::thrift
