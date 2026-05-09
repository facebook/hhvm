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

#include <folly/GLog.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/util/ManagedStringView.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Construct RequestRpcMetadata with RpcOptions. Returns the unsealized
 * struct — the transport adapter serializes it as part of
 * `ThriftRequestResponsePayload::toRocketFrame()`.
 */
inline std::unique_ptr<apache::thrift::RequestRpcMetadata> makeRequestMetadata(
    const apache::thrift::RpcOptions& rpcOptions,
    apache::thrift::ManagedStringView methodName,
    apache::thrift::RpcKind rpcKind,
    apache::thrift::ProtocolId protocolId) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->protocol() = protocolId;
  metadata->kind() = rpcKind;
  metadata->name() =
      apache::thrift::ManagedStringViewWithConversions(std::move(methodName));

  if (auto timeout = rpcOptions.getTimeout();
      timeout > std::chrono::milliseconds::zero()) {
    metadata->clientTimeoutMs() = timeout.count();
  }
  if (auto queueTimeout = rpcOptions.getQueueTimeout();
      queueTimeout > std::chrono::milliseconds::zero()) {
    metadata->queueTimeoutMs() = queueTimeout.count();
  }
  if (rpcOptions.getPriority() < apache::thrift::concurrency::N_PRIORITIES) {
    metadata->priority() =
        static_cast<apache::thrift::RpcPriority>(rpcOptions.getPriority());
  }

  // Translate RpcOptions::Checksum into the corresponding metadata field.
  // Only XXH3_64 is supported; CRC32 and SERVER_ONLY_CRC32 are accepted by
  // the public RpcOptions API but treated as no-ops here — fast_thrift does
  // not support CRC-family checksums. The value is left as a placeholder (0);
  // ThriftClientChecksumHandler computes it over `data` later in the
  // pipeline, before the transport adapter serializes the metadata.
  switch (rpcOptions.getChecksum()) {
    case apache::thrift::RpcOptions::Checksum::XXH3_64: {
      apache::thrift::Checksum c;
      c.algorithm() = apache::thrift::ChecksumAlgorithm::XXH3_64;
      metadata->checksum() = c;
      break;
    }
    case apache::thrift::RpcOptions::Checksum::CRC32:
      FB_LOG_ONCE(ERROR)
          << "RpcOptions::Checksum::CRC32 is not supported by fast_thrift; "
             "use XXH3_64. Treating as NONE.";
      break;
    case apache::thrift::RpcOptions::Checksum::SERVER_ONLY_CRC32:
      FB_LOG_ONCE(ERROR)
          << "RpcOptions::Checksum::SERVER_ONLY_CRC32 is not supported by "
             "fast_thrift; use XXH3_64. Treating as NONE.";
      break;
    case apache::thrift::RpcOptions::Checksum::NONE:
      break;
    default:
      FB_LOG_ONCE(ERROR) << "Unexpected RpcOptions::Checksum value: "
                         << static_cast<int>(rpcOptions.getChecksum())
                         << ". Treating as NONE.";
      break;
  }

  return metadata;
}

/**
 * Construct RequestRpcMetadata without RpcOptions.
 */
inline std::unique_ptr<apache::thrift::RequestRpcMetadata> makeRequestMetadata(
    apache::thrift::ManagedStringView methodName,
    apache::thrift::RpcKind rpcKind,
    apache::thrift::ProtocolId protocolId) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->protocol() = protocolId;
  metadata->kind() = rpcKind;
  metadata->name() =
      apache::thrift::ManagedStringViewWithConversions(std::move(methodName));
  return metadata;
}

} // namespace apache::thrift::fast_thrift::thrift
