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
#include <string>
#include <utility>

#include <folly/Conv.h>

#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

// Controls the rollout of defaulting the CUSTOM compression fallback to zstd.
// When enabled, a CUSTOM codec that has not yet been negotiated falls back to
// zstd; when disabled, it falls back to zlib (the legacy behavior, which is
// then upgraded to zstd only once serverZstdSupported is known).
THRIFT_FLAG_DEFINE_bool(
    thrift_client_custom_compression_fallback_to_zstd, true);

// Controls whether a client downgrades LZ4 to zlib when the peer has not
// advertised LZ4 support. Disabled by default and intended to be enabled only
// once SetupResponse.lz4Supported has propagated across the server fleet:
// enabling it earlier would treat every not-yet-rebuilt server as LZ4-incapable
// at once, reverting the LZ4 rollout (T278225134) to zlib fleet-wide.
THRIFT_FLAG_DEFINE_bool(thrift_client_lz4_downgrade_when_unsupported, false);

namespace apache::thrift::detail {

RequestRpcMetadata makeRequestRpcMetadata(
    const RpcOptions& rpcOptions,
    RpcKind kind,
    MethodMetadata&& methodMetadata,
    std::optional<std::chrono::milliseconds> clientTimeout,
    std::variant<InteractionCreate, int64_t, std::monostate> interactionHandle,
    bool serverZstdSupported,
    bool serverLz4Supported,
    ssize_t payloadSize,
    transport::THeader& header,
    std::unique_ptr<folly::IOBuf> interceptorFrameworkMetadata,
    bool customCompressionEnabled) {
  auto methodName = methodMetadata.name_managed();

  RequestRpcMetadata metadata;
  metadata.protocol() = static_cast<ProtocolId>(header.getProtocolId());
  metadata.kind() = kind;
  metadata.name() = ManagedStringViewWithConversions(std::move(methodName));

  if (!rpcOptions.getClientOnlyTimeouts()) {
    if (clientTimeout.has_value()) {
      metadata.clientTimeoutMs() = clientTimeout->count();
    }
    if (rpcOptions.getQueueTimeout() > std::chrono::milliseconds::zero()) {
      metadata.queueTimeoutMs() = rpcOptions.getQueueTimeout().count();
    }
  }

  if (rpcOptions.getPriority() < concurrency::N_PRIORITIES) {
    metadata.priority() = static_cast<RpcPriority>(rpcOptions.getPriority());
  }
  if (header.getCrc32c().has_value()) {
    metadata.crc32c() = header.getCrc32c().value();
  }
  // add user specified compression settings to metadata
  if (auto compressionConfig = header.getDesiredCompressionConfig()) {
    if (auto codec = compressionConfig->codecConfig()) {
      // Custom codec cannot be used until negotiation completes (e.g. the first
      // request on a connection). Fall back to zstd: custom compression is
      // Rocket-only and every Rocket server unconditionally supports zstd
      // decompression (setup response always has zstdSupported=true), so zstd
      // is safe even before serverZstdSupported is known on this side. The
      // legacy zlib fallback (auto-upgraded to zstd only once
      // serverZstdSupported is known) is kept behind a flag to allow rolling
      // back the zstd default.
      if (codec->getType() == CodecConfig::Type::customConfig &&
          !customCompressionEnabled) {
        if (THRIFT_FLAG(thrift_client_custom_compression_fallback_to_zstd)) {
          codec->zstdConfig().emplace();
        } else {
          codec->zlibConfig().emplace();
        }
      }

      // Downgrade LZ4 to zlib when the peer has not advertised that it can
      // decode LZ4. A server lacking the codec may drop the request without
      // replying, which the caller sees as RECV_TIMEOUT rather than
      // INVALID_TRANSFORM, so the failure is not attributable after the fact
      // and the advertised capability is the only usable signal.
      //
      // Must stay ABOVE the zlib->zstd upgrade below, which then carries the
      // downgrade on to zstd whenever the peer supports it. That reproduces
      // what the service had before the LZ4 migration: a zlib-configured
      // service was already upgraded to zstd on any zstd-capable peer, so zlib
      // remains the final codec only for a peer advertising neither.
      if (THRIFT_FLAG(thrift_client_lz4_downgrade_when_unsupported) &&
          codec->getType() == CodecConfig::Type::lz4Config &&
          !serverLz4Supported) {
        codec->zlibConfig().emplace();
      }

      if (codec->getType() == CodecConfig::Type::zlibConfig &&
          serverZstdSupported) {
        codec->zstdConfig().emplace();
      }

      if (payloadSize > compressionConfig->compressionSizeLimit().value_or(0)) {
        metadata.compression() =
            rocket::CompressionManager().fromCodecConfig(*codec);
      }

      metadata.compressionConfig() = *compressionConfig;
    }
  }

  if (rpcOptions.getChecksum() == RpcOptions::Checksum::CRC32) {
    Checksum checksum;
    checksum.algorithm() = ChecksumAlgorithm::CRC32;
    metadata.checksum() = checksum;
  } else if (rpcOptions.getChecksum() == RpcOptions::Checksum::XXH3_64) {
    Checksum checksum;
    checksum.algorithm() = ChecksumAlgorithm::XXH3_64;
    metadata.checksum() = checksum;
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
    metadata.clientId() = *clientId;
  }

  if (const auto& serviceTraceMeta = header.serviceTraceMeta()) {
    metadata.serviceTraceMeta() = *serviceTraceMeta;
  }

  if (const auto& tenantId = header.tenantId()) {
    metadata.tenantId() = *tenantId;
  }

  auto loadIt = writeHeaders.find(transport::THeader::QUERY_LOAD_HEADER);
  if (loadIt != writeHeaders.end()) {
    metadata.loadMetric() = std::move(loadIt->second);
    writeHeaders.erase(loadIt);
  }

  auto secLoadIt =
      writeHeaders.find(transport::THeader::QUERY_SECONDARY_LOAD_HEADER);
  if (secLoadIt != writeHeaders.end()) {
    metadata.secondaryLoadMetric() = std::move(secLoadIt->second);
    writeHeaders.erase(secLoadIt);
  }

  auto stopperMetricIt =
      writeHeaders.find(transport::THeader::QUERY_STOPPER_METRIC);
  if (stopperMetricIt != writeHeaders.end()) {
    metadata.stopperMetric() = std::move(stopperMetricIt->second);
    writeHeaders.erase(stopperMetricIt);
  }

  auto grLoadIt =
      writeHeaders.find(transport::THeader::QUERY_GLOBAL_ROUTING_LOAD_HEADER);
  if (grLoadIt != writeHeaders.end()) {
    metadata.grLoadMetric() = std::move(grLoadIt->second);
    writeHeaders.erase(grLoadIt);
  }

  auto grSecondaryLoadIt = writeHeaders.find(
      transport::THeader::QUERY_GLOBAL_ROUTING_SECONDARY_LOAD_HEADER);
  if (grSecondaryLoadIt != writeHeaders.end()) {
    metadata.grSecondaryLoadMetric() = std::move(grSecondaryLoadIt->second);
    writeHeaders.erase(grSecondaryLoadIt);
  }

  auto grHealthIt =
      writeHeaders.find(transport::THeader::QUERY_GLOBAL_ROUTING_HEALTH_HEADER);
  if (grHealthIt != writeHeaders.end()) {
    metadata.grHealthMetric() = std::move(grHealthIt->second);
    writeHeaders.erase(grHealthIt);
  }

  if (!writeHeaders.empty()) {
    metadata.otherMetadata() = std::move(writeHeaders);
  }

  bool isFromInterceptor = interceptorFrameworkMetadata != nullptr;

  if (isFromInterceptor) {
    metadata.frameworkMetadata() = std::move(interceptorFrameworkMetadata);
  } else if (rpcOptions.getContextPropMask()) {
    folly::dynamic logMessages = folly::dynamic::object();
    auto frameworkMetadata = makeFrameworkMetadata(rpcOptions, logMessages);
    if (frameworkMetadata) {
      metadata.frameworkMetadata() = std::move(frameworkMetadata);
    }
    if (!logMessages.empty()) {
      THRIFT_APPLICATION_EVENT(framework_metadata_construction).log([&] {
        return logMessages;
      });
    }
  }

  if (const auto& fmd = metadata.frameworkMetadata().as_const()) {
    THRIFT_APPLICATION_EVENT(rpc_metadata_framework_metadata).log([&] {
      folly::dynamic log = folly::dynamic::object;

      log["service_name"] =
          methodMetadata.thriftServiceUriOrName_managed().str(),
      log["method_name"] = metadata.name().value().str(),
      log["is_from_interceptor"] = isFromInterceptor ? 1 : 0;
      log["framework_metadata_size"] =
          fmd.value().get()->computeChainDataLength();

      return log;
    });
  }

  if (const auto& loggingContext = header.loggingContext()) {
    metadata.loggingContext() = *loggingContext;
  }

  if (const auto& quotaReportConfig = header.quotaReportConfig()) {
    metadata.quotaReportConfig() = *quotaReportConfig;
  }

  if (std::holds_alternative<InteractionCreate>(interactionHandle)) {
    metadata.interactionCreate() =
        std::get<InteractionCreate>(interactionHandle);
  } else if (std::holds_alternative<int64_t>(interactionHandle)) {
    metadata.interactionId() = std::get<int64_t>(interactionHandle);
  }

  return metadata;
}

void fillTHeaderFromResponseRpcMetadata(
    ResponseRpcMetadata& responseMetadata, transport::THeader& header) {
  if (responseMetadata.otherMetadata()) {
    header.setReadHeaders(std::move(*responseMetadata.otherMetadata()));
  }
  if (auto load = responseMetadata.load()) {
    header.setServerLoad(*load);
    header.setReadHeader(
        transport::THeader::QUERY_LOAD_HEADER, folly::to<std::string>(*load));
  }

  if (auto load = responseMetadata.secondaryLoad()) {
    header.setServerSecondaryLoad(*load);
    header.setReadHeader(
        transport::THeader::QUERY_SECONDARY_LOAD_HEADER,
        folly::to<std::string>(*load));
  }

  if (auto stopperMetric = responseMetadata.stopperMetric()) {
    header.setStopperMetricValue(*stopperMetric);
    header.setReadHeader(
        transport::THeader::QUERY_STOPPER_METRIC,
        folly::to<std::string>(*stopperMetric));
  }

  if (auto grLoad = responseMetadata.grLoad()) {
    header.setGrLoadValue(*grLoad);
    header.setReadHeader(
        transport::THeader::QUERY_GLOBAL_ROUTING_LOAD_HEADER,
        folly::to<std::string>(*grLoad));
  }

  if (auto grSecondaryLoad = responseMetadata.grSecondaryLoad()) {
    header.setGrSecondaryLoadValue(*grSecondaryLoad);
    header.setReadHeader(
        transport::THeader::QUERY_GLOBAL_ROUTING_SECONDARY_LOAD_HEADER,
        folly::to<std::string>(*grSecondaryLoad));
  }

  if (auto grHealth = responseMetadata.grHealth()) {
    header.setGrHealthValue(*grHealth);
    header.setReadHeader(
        transport::THeader::QUERY_GLOBAL_ROUTING_HEALTH_HEADER,
        folly::to<std::string>(*grHealth));
  }

  if (auto crc32c = responseMetadata.crc32c()) {
    header.setCrc32c(*crc32c);
  }

  if (auto checksum = responseMetadata.checksum()) {
    header.setChecksum(*checksum);
  }

  if (auto compression = responseMetadata.compression()) {
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
  if (auto queueMetadata = responseMetadata.queueMetadata()) {
    header.setProcessDelay(
        std::chrono::milliseconds(*queueMetadata->queueingTimeMs()));
    if (auto queueTimeout = queueMetadata->queueTimeoutMs()) {
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
      responseMetadata.load() = folly::to<int64_t>(loadIt->second);
      otherMetadata.erase(loadIt);
    }
  }
  {
    auto secLoadIt =
        otherMetadata.find(transport::THeader::QUERY_SECONDARY_LOAD_HEADER);
    if (secLoadIt != otherMetadata.end()) {
      responseMetadata.secondaryLoad() = folly::to<int64_t>(secLoadIt->second);
      otherMetadata.erase(secLoadIt);
    }
  }
  {
    auto stopperMetricIt =
        otherMetadata.find(transport::THeader::QUERY_STOPPER_METRIC);
    if (stopperMetricIt != otherMetadata.end()) {
      responseMetadata.stopperMetric() =
          folly::to<int64_t>(stopperMetricIt->second);
      otherMetadata.erase(stopperMetricIt);
    }
  }
  {
    auto grLoadIt = otherMetadata.find(
        transport::THeader::QUERY_GLOBAL_ROUTING_LOAD_HEADER);
    if (grLoadIt != otherMetadata.end()) {
      responseMetadata.grLoad() = folly::to<int64_t>(grLoadIt->second);
      otherMetadata.erase(grLoadIt);
    }
  }
  {
    auto grSecondaryLoadIt = otherMetadata.find(
        transport::THeader::QUERY_GLOBAL_ROUTING_SECONDARY_LOAD_HEADER);
    if (grSecondaryLoadIt != otherMetadata.end()) {
      responseMetadata.grSecondaryLoad() =
          folly::to<int64_t>(grSecondaryLoadIt->second);
      otherMetadata.erase(grSecondaryLoadIt);
    }
  }
  {
    auto grHealthIt = otherMetadata.find(
        transport::THeader::QUERY_GLOBAL_ROUTING_HEALTH_HEADER);
    if (grHealthIt != otherMetadata.end()) {
      responseMetadata.grHealth() = folly::to<int64_t>(grHealthIt->second);
      otherMetadata.erase(grHealthIt);
    }
  }
  if (auto crc32c = header.getCrc32c()) {
    responseMetadata.crc32c() = *crc32c;
  }
  responseMetadata.otherMetadata() = std::move(otherMetadata);
  if (auto checksum = responseMetadata.checksum()) {
    header.setChecksum(*checksum);
  }
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

} // namespace apache::thrift::detail
