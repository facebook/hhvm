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

#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>

#include <chrono>
#include <map>
#include <string>
#include <utility>

#include <folly/Conv.h>

#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::detail {

RequestRpcMetadata makeRequestRpcMetadata(
    const RpcOptions& rpcOptions,
    RpcKind kind,
    ProtocolId protocolId,
    ManagedStringView&& methodName,
    std::chrono::milliseconds defaultChannelTimeout,
    transport::THeader& header) {
  RequestRpcMetadata metadata;
  metadata.protocol_ref() = protocolId;
  metadata.kind_ref() = kind;
  metadata.name_ref() = ManagedStringViewWithConversions(std::move(methodName));
  if (rpcOptions.getTimeout() > std::chrono::milliseconds::zero()) {
    metadata.clientTimeoutMs_ref() = rpcOptions.getTimeout().count();
  } else if (defaultChannelTimeout > std::chrono::milliseconds::zero()) {
    metadata.clientTimeoutMs_ref() = defaultChannelTimeout.count();
  }
  if (rpcOptions.getQueueTimeout() > std::chrono::milliseconds::zero()) {
    metadata.queueTimeoutMs_ref() = rpcOptions.getQueueTimeout().count();
  }
  if (rpcOptions.getPriority() < concurrency::N_PRIORITIES) {
    metadata.priority_ref() =
        static_cast<RpcPriority>(rpcOptions.getPriority());
  }
  if (header.getCrc32c().has_value()) {
    metadata.crc32c_ref() = header.getCrc32c().value();
  }
  // add user specified compression settings to metadata
  if (auto compressionConfig = header.getDesiredCompressionConfig()) {
    metadata.compressionConfig_ref() = *compressionConfig;
  }

  auto writeHeaders = header.releaseWriteHeaders();
  if (auto* eh = header.getExtraWriteHeaders()) {
    // Extra write headers always take precedence over write headers (see
    // THeader.cpp). We must copy here since we don't own the extra write
    // headers.
    for (const auto& entry : *eh) {
      writeHeaders[entry.first] = entry.second;
    }
  }

  if (const auto& clientId = header.clientId()) {
    metadata.clientId_ref() = *clientId;
  }

  if (const auto& serviceTraceMeta = header.serviceTraceMeta()) {
    metadata.serviceTraceMeta_ref() = *serviceTraceMeta;
  }

  if (const auto& tenantId = header.tenantId()) {
    metadata.tenantId_ref() = *tenantId;
  }

  auto loadIt = writeHeaders.find(transport::THeader::QUERY_LOAD_HEADER);
  if (loadIt != writeHeaders.end()) {
    metadata.loadMetric_ref() = std::move(loadIt->second);
    writeHeaders.erase(loadIt);
  }

  if (!writeHeaders.empty()) {
    metadata.otherMetadata_ref() = std::move(writeHeaders);
  }

  if (rpcOptions.getContextPropMask()) {
    folly::dynamic logMessages = folly::dynamic::object();
    auto frameworkMetadata = makeFrameworkMetadata(
        rpcOptions, logMessages, metadata.otherMetadata_ref().ensure());
    if (frameworkMetadata) {
      metadata.frameworkMetadata_ref() = std::move(frameworkMetadata);
    }
    if (!logMessages.empty()) {
      THRIFT_APPLICATION_EVENT(framework_metadata_construction).log([&] {
        return logMessages;
      });
    }
  }

  if (const auto& loggingContext = header.loggingContext()) {
    metadata.loggingContext() = *loggingContext;
  }

  return metadata;
}

void fillTHeaderFromResponseRpcMetadata(
    ResponseRpcMetadata& responseMetadata, transport::THeader& header) {
  if (responseMetadata.otherMetadata_ref()) {
    header.setReadHeaders(std::move(*responseMetadata.otherMetadata_ref()));
  }
  if (auto load = responseMetadata.load_ref()) {
    header.setServerLoad(*load);
    header.setReadHeader(
        transport::THeader::QUERY_LOAD_HEADER, folly::to<std::string>(*load));
  }
  if (auto crc32c = responseMetadata.crc32c_ref()) {
    header.setCrc32c(*crc32c);
  }
  if (auto compression = responseMetadata.compression_ref()) {
    // for fb internal logging purpose only; does not actually do transformation
    // based on THeader
    transport::THeader::TRANSFORMS transform;
    switch (*compression) {
      case apache::thrift::CompressionAlgorithm::ZSTD:
        transform = transport::THeader::ZSTD_TRANSFORM;
        break;
      case apache::thrift::CompressionAlgorithm::ZLIB:
        transform = transport::THeader::ZLIB_TRANSFORM;
        break;
      default:
        transform = transport::THeader::NONE;
        break;
    }
    if (transform != transport::THeader::NONE) {
      header.setReadTransform(static_cast<uint16_t>(transform));
    }
  }
  if (auto queueMetadata = responseMetadata.queueMetadata_ref()) {
    header.setProcessDelay(
        std::chrono::milliseconds(*queueMetadata->queueingTimeMs_ref()));
    if (auto queueTimeout = queueMetadata->queueTimeoutMs_ref()) {
      header.setServerQueueTimeout(std::chrono::milliseconds(*queueTimeout));
    }
  }
}

void fillResponseRpcMetadataFromTHeader(
    transport::THeader& header, ResponseRpcMetadata& responseMetadata) {
  auto otherMetadata = header.releaseHeaders();
  {
    auto loadIt = otherMetadata.find(transport::THeader::QUERY_LOAD_HEADER);
    if (loadIt != otherMetadata.end()) {
      responseMetadata.load_ref() = folly::to<int64_t>(loadIt->second);
      otherMetadata.erase(loadIt);
    }
  }
  if (auto crc32c = header.getCrc32c()) {
    responseMetadata.crc32c_ref() = *crc32c;
  }
  responseMetadata.otherMetadata_ref() = std::move(otherMetadata);
}

std::string serializeErrorClassification(ErrorClassification ec) {
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(ec);
  return protocol::base64Encode(folly::StringPiece(serialized));
}

ErrorClassification deserializeErrorClassification(std::string_view str) {
  auto buf = protocol::base64Decode(str);
  return CompactSerializer::deserialize<ErrorClassification>(buf.get());
}

// Make these in sync with thrift/lib/thrift/RpcMetadata.thrift
folly::Optional<std::string> errorKindToString(ErrorKind kind) {
  switch (kind) {
    case ErrorKind::TRANSIENT:
      return std::string("TRANSIENT");
    case ErrorKind::STATEFUL:
      return std::string("STATEFUL");
    case ErrorKind::PERMANENT:
      return std::string("PERMANENT");
    default:
      return folly::none;
  }
}

folly::Optional<std::string> errorBlameToString(ErrorBlame blame) {
  switch (blame) {
    case ErrorBlame::SERVER:
      return std::string("SERVER");
    case ErrorBlame::CLIENT:
      return std::string("CLIENT");
    default:
      return folly::none;
  }
}

folly::Optional<std::string> errorSafetyToString(ErrorSafety safety) {
  switch (safety) {
    case ErrorSafety::SAFE:
      return std::string("SAFE");
    default:
      return folly::none;
  }
}

} // namespace apache::thrift::detail
