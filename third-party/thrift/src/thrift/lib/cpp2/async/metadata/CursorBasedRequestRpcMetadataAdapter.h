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

#include <thrift/lib/cpp2/async/metadata/RequestRpcMetadataAdapter.h>
#include <thrift/lib/cpp2/protocol/CursorBasedSerializer.h>

namespace apache::thrift {

class CursorBasedRequestRpcMetadataAdapter final {
 public:
  FOLLY_ERASE explicit CursorBasedRequestRpcMetadataAdapter(
      std::unique_ptr<folly::IOBuf>&& serializedMetadata) {
    init(std::move(serializedMetadata));
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<ProtocolId>> protocolId() {
    if (protocolId_.has_value()) {
      return protocolId_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<ManagedStringViewField>>
  name() {
    if (name_.has_value()) {
      return name_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<RpcKind>> kind() {
    if (kind_.has_value()) {
      return kind_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> clientTimeoutMs() {
    if (clientTimeoutMs_.has_value()) {
      return clientTimeoutMs_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> queueTimeoutMs() {
    if (queueTimeoutMs_.has_value()) {
      return queueTimeoutMs_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<RpcPriority>> priority() {
    if (priority_.has_value()) {
      return priority_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<folly::F14NodeMap<std::string, std::string>*>
  otherMetadata() {
    if (otherMetadata_.has_value()) {
      return &*otherMetadata_;
    } else {
      return std::nullopt;
    }
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<uint32_t>> crc32c() {
    if (crc32c_.has_value()) {
      return crc32c_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> loadMetric() {
    if (loadMetric_.has_value()) {
      return loadMetric_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionAlgorithm>>
  compression() {
    if (compression_.has_value()) {
      return compression_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionConfig>>
  compressionConfig() {
    if (compressionConfig_.has_value()) {
      return compressionConfig_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<int64_t>> interactionId() {
    if (interactionId_.has_value()) {
      return interactionId_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<InteractionCreate>>
  interactionCreate() {
    if (interactionCreate_.has_value()) {
      return interactionCreate_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> clientId() {
    if (clientId_.has_value()) {
      return clientId_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>>
  serviceTraceMeta() {
    if (serviceTraceMeta_.has_value()) {
      return serviceTraceMeta_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE
  std::optional<std::reference_wrapper<std::unique_ptr<folly::IOBuf>>>
  frameworkMetadata() {
    if (frameworkMetadata_.has_value()) {
      return frameworkMetadata_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<FdMetadata>> fdMetadata() {
    if (fdMetadata_.has_value()) {
      return fdMetadata_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<LoggingContext>>
  loggingContext() {
    if (loggingContext_.has_value()) {
      return loggingContext_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> tenantId() {
    if (tenantId_.has_value()) {
      return tenantId_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<Checksum>> checksum() {
    if (checksum_.has_value()) {
      return checksum_.value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<bool>> checksumResponse() {
    if (checksumResponse_.has_value()) {
      return checksumResponse_.value();
    }
    return std::nullopt;
  }

 private:
  std::optional<ProtocolId> protocolId_;
  std::optional<ManagedStringViewField> name_;
  std::optional<RpcKind> kind_;
  std::optional<int32_t> clientTimeoutMs_;
  std::optional<int32_t> queueTimeoutMs_;
  std::optional<RpcPriority> priority_;
  std::optional<folly::F14NodeMap<std::string, std::string>> otherMetadata_;
  std::optional<uint32_t> crc32c_;
  std::optional<std::string> loadMetric_;
  std::optional<CompressionAlgorithm> compression_;
  std::optional<CompressionConfig> compressionConfig_;
  std::optional<int64_t> interactionId_;
  std::optional<InteractionCreate> interactionCreate_;
  std::optional<std::string> clientId_;
  std::optional<std::string> serviceTraceMeta_;
  std::optional<std::unique_ptr<folly::IOBuf>> frameworkMetadata_;
  std::optional<FdMetadata> fdMetadata_;
  std::optional<LoggingContext> loggingContext_;
  std::optional<std::string> tenantId_;
  std::optional<Checksum> checksum_;
  std::optional<bool> checksumResponse_;

  void init(std::unique_ptr<folly::IOBuf>&& serializedMetadata) {
    DCHECK(serializedMetadata);
    CursorSerializationWrapper<RequestRpcMetadata> cursor(
        std::move(serializedMetadata));
    auto reader = cursor.beginRead();
    protocolId_ = reader.read<ident::protocol>();
    name_ = reader.read<ident::name>();
    kind_ = reader.read<ident::kind>();
    clientTimeoutMs_ = reader.read<ident::clientTimeoutMs>();
    queueTimeoutMs_ = reader.read<ident::queueTimeoutMs>();
    priority_ = reader.read<ident::priority>();
    otherMetadata_ = reader.read<ident::otherMetadata>();
    crc32c_ = reader.read<ident::crc32c>();
    loadMetric_ = reader.read<ident::loadMetric>();
    compression_ = reader.read<ident::compression>();
    compressionConfig_ = reader.read<ident::compressionConfig>();
    interactionId_ = reader.read<ident::interactionId>();
    interactionCreate_ = reader.read<ident::interactionCreate>();
    clientId_ = reader.read<ident::clientId>();
    serviceTraceMeta_ = reader.read<ident::serviceTraceMeta>();
    frameworkMetadata_ = reader.read<ident::frameworkMetadata>();
    fdMetadata_ = reader.read<ident::fdMetadata>();
    loggingContext_ = reader.read<ident::loggingContext>();
    tenantId_ = reader.read<ident::tenantId>();
    checksum_ = reader.read<ident::checksum>();
    checksumResponse_ = reader.read<ident::checksumResponse>();
    cursor.endRead(std::move(reader));
  }
};

#if __cpluscplus >= 202002L
static_assert(requires {
  requires RequestRpcMetadataAdapter<CursorBasedRequestRpcMetadataAdapter>;
});
#endif

} // namespace apache::thrift
