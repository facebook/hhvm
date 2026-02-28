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

#include <type_traits>
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
    return visit([](auto& adapter) { return adapter.protocolId(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<ManagedStringViewField>>
  name() {
    return visit([](auto& adapter) { return adapter.name(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<RpcKind>> kind() {
    return visit([](auto& adapter) { return adapter.kind(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> clientTimeoutMs() {
    return visit([](auto& adapter) { return adapter.clientTimeoutMs(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> queueTimeoutMs() {
    return visit([](auto& adapter) { return adapter.queueTimeoutMs(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<RpcPriority>> priority() {
    return visit([](auto& adapter) { return adapter.priority(); });
  }
  FOLLY_ERASE std::optional<folly::F14NodeMap<std::string, std::string>*>
  otherMetadata() {
    return visit([](auto& adapter) { return adapter.otherMetadata(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<uint32_t>> crc32c() {
    return visit([](auto& adapter) { return adapter.crc32c(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> loadMetric() {
    return visit([](auto& adapter) { return adapter.loadMetric(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionAlgorithm>>
  compression() {
    return visit([](auto& adapter) { return adapter.compression(); });
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionConfig>>
  compressionConfig() {
    return visit([](auto& adapter) { return adapter.compressionConfig(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<int64_t>> interactionId() {
    return visit([](auto& adapter) { return adapter.interactionId(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<InteractionCreate>>
  interactionCreate() {
    return visit([](auto& adapter) { return adapter.interactionCreate(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> clientId() {
    return visit([](auto& adapter) { return adapter.clientId(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>>
  serviceTraceMeta() {
    return visit([](auto& adapter) { return adapter.serviceTraceMeta(); });
  }
  FOLLY_ERASE
  std::optional<std::reference_wrapper<std::unique_ptr<folly::IOBuf>>>
  frameworkMetadata() {
    return visit([](auto& adapter) { return adapter.frameworkMetadata(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<FdMetadata>> fdMetadata() {
    return visit([](auto& adapter) { return adapter.fdMetadata(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<LoggingContext>>
  loggingContext() {
    return visit([](auto& adapter) { return adapter.loggingContext(); });
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> tenantId() {
    return visit([](auto& adapter) { return adapter.tenantId(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<Checksum>> checksum() {
    return visit([](auto& adapter) { return adapter.checksum(); });
  }
  FOLLY_ERASE std::optional<std::reference_wrapper<bool>> checksumResponse() {
    return visit([](auto& adapter) { return adapter.checksumResponse(); });
  }

 private:
  std::variant<
      TCompactRequestRpcMetadataAdapter,
      CursorBasedRequestRpcMetadataAdapter>
      adapter_;

  template <typename DelegateFunc>
  using DelegateReturn = std::common_type_t<
      std::invoke_result_t<DelegateFunc, TCompactRequestRpcMetadataAdapter&>,
      std::
          invoke_result_t<DelegateFunc, CursorBasedRequestRpcMetadataAdapter&>>;

  template <
      typename DelegateFunc,
      typename ReturnType = DelegateReturn<DelegateFunc&>>
  FOLLY_ALWAYS_INLINE ReturnType visit(DelegateFunc&& delegate) {
    if (std::holds_alternative<TCompactRequestRpcMetadataAdapter>(adapter_)) {
      auto& adapter = std::get<TCompactRequestRpcMetadataAdapter>(adapter_);
      return delegate(adapter);
    } else {
      auto& adapter = std::get<CursorBasedRequestRpcMetadataAdapter>(adapter_);
      return delegate(adapter);
    }
  }
};

FOLLY_ERASE RequestRpcMetadataFacade::RequestRpcMetadataFacade(
    std::unique_ptr<folly::IOBuf>&& serializedMetadata)
    : adapter_(
          std::variant<
              TCompactRequestRpcMetadataAdapter,
              CursorBasedRequestRpcMetadataAdapter>(
              std::in_place_type<TCompactRequestRpcMetadataAdapter>,
              std::move(serializedMetadata))) {}

FOLLY_ERASE RequestRpcMetadataFacade::RequestRpcMetadataFacade(
    folly::IOBuf& serializedMetadata)
    : adapter_(
          std::variant<
              TCompactRequestRpcMetadataAdapter,
              CursorBasedRequestRpcMetadataAdapter>(
              std::in_place_type<TCompactRequestRpcMetadataAdapter>,
              serializedMetadata)) {}

} // namespace apache::thrift
