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

#include <memory>
#include <thrift/lib/cpp2/async/metadata/RequestRpcMetadataAdapter.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift {

class TCompactRequestRpcMetadataAdapter final {
 public:
  FOLLY_ERASE explicit TCompactRequestRpcMetadataAdapter(
      std::unique_ptr<folly::IOBuf> serializedMetadata) {
    DCHECK(serializedMetadata);
    CompactProtocolReader reader;
    reader.setInput(serializedMetadata.get());
    requestMetadata_.read(&reader);
  }

  FOLLY_ERASE explicit TCompactRequestRpcMetadataAdapter(
      folly::IOBuf& serializedMetadata)
      : requestMetadata_([&] {
          RequestRpcMetadata out;
          CompactProtocolReader reader;
          reader.setInput(&serializedMetadata);
          out.read(&reader);
          return out;
        }()) {}

  FOLLY_ERASE std::optional<std::reference_wrapper<ProtocolId>> protocolId() {
    if (requestMetadata_.protocol().has_value()) {
      return requestMetadata_.protocol().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<ManagedStringViewField>>
  name() {
    if (requestMetadata_.name().has_value()) {
      return requestMetadata_.name().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<RpcKind>> kind() {
    if (requestMetadata_.kind().has_value()) {
      return requestMetadata_.kind().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> clientTimeoutMs() {
    if (requestMetadata_.clientTimeoutMs().has_value()) {
      return requestMetadata_.clientTimeoutMs().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<int32_t>> queueTimeoutMs() {
    if (requestMetadata_.queueTimeoutMs().has_value()) {
      return requestMetadata_.queueTimeoutMs().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<RpcPriority>> priority() {
    if (requestMetadata_.priority().has_value()) {
      return requestMetadata_.priority().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<folly::F14NodeMap<std::string, std::string>*>
  otherMetadata() {
    if (requestMetadata_.otherMetadata().has_value()) {
      return &*requestMetadata_.otherMetadata();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<uint32_t>> crc32c() {
    if (requestMetadata_.crc32c().has_value()) {
      return requestMetadata_.crc32c().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> loadMetric() {
    if (requestMetadata_.loadMetric().has_value()) {
      return requestMetadata_.loadMetric().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionAlgorithm>>
  compression() {
    if (requestMetadata_.compression().has_value()) {
      return requestMetadata_.compression().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<CompressionConfig>>
  compressionConfig() {
    if (requestMetadata_.compressionConfig().has_value()) {
      return requestMetadata_.compressionConfig().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<int64_t>> interactionId() {
    if (requestMetadata_.interactionId().has_value()) {
      return requestMetadata_.interactionId().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<InteractionCreate>>
  interactionCreate() {
    if (requestMetadata_.interactionCreate().has_value()) {
      return requestMetadata_.interactionCreate().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> clientId() {
    if (requestMetadata_.clientId().has_value()) {
      return requestMetadata_.clientId().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>>
  serviceTraceMeta() {
    if (requestMetadata_.serviceTraceMeta().has_value()) {
      return requestMetadata_.serviceTraceMeta().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE
  std::optional<std::reference_wrapper<std::unique_ptr<folly::IOBuf>>>
  frameworkMetadata() {
    if (requestMetadata_.frameworkMetadata().has_value()) {
      return requestMetadata_.frameworkMetadata().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<FdMetadata>> fdMetadata() {
    if (requestMetadata_.fdMetadata().has_value()) {
      return requestMetadata_.fdMetadata().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<LoggingContext>>
  loggingContext() {
    if (requestMetadata_.loggingContext().has_value()) {
      return requestMetadata_.loggingContext().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<std::string>> tenantId() {
    if (requestMetadata_.tenantId().has_value()) {
      return requestMetadata_.tenantId().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<Checksum>> checksum() {
    if (requestMetadata_.checksum().has_value()) {
      return requestMetadata_.checksum().value();
    }
    return std::nullopt;
  }

  FOLLY_ERASE std::optional<std::reference_wrapper<bool>> checksumResponse() {
    if (requestMetadata_.checksumResponse().has_value()) {
      return requestMetadata_.checksumResponse().value();
    }
    return std::nullopt;
  }

 private:
  RequestRpcMetadata requestMetadata_;
};

#if __cpluscplus >= 202002L
static_assert(requires {
  requires RequestRpcMetadataAdapter<TCompactRequestRpcMetadataAdapter>;
});
#endif

} // namespace apache::thrift
