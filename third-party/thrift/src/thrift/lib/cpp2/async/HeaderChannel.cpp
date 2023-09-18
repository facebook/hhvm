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

#include <thrift/lib/cpp2/async/HeaderChannel.h>

#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache {
namespace thrift {

using apache::thrift::transport::THeader;

void HeaderChannel::addRpcOptionHeaders(
    THeader* header, const RpcOptions& rpcOptions) {
  if (!clientSupportHeader()) {
    return;
  }

  if (rpcOptions.getPriority() != apache::thrift::concurrency::N_PRIORITIES) {
    header->setHeader(
        transport::THeader::PRIORITY_HEADER,
        folly::to<std::string>(rpcOptions.getPriority()));
  }

  if (!rpcOptions.getClientOnlyTimeouts()) {
    if (rpcOptions.getTimeout() > std::chrono::milliseconds(0)) {
      header->setHeader(
          transport::THeader::CLIENT_TIMEOUT_HEADER,
          folly::to<std::string>(rpcOptions.getTimeout().count()));
    }

    if (rpcOptions.getQueueTimeout() > std::chrono::milliseconds(0)) {
      header->setHeader(
          transport::THeader::QUEUE_TIMEOUT_HEADER,
          folly::to<std::string>(rpcOptions.getQueueTimeout().count()));
    }

    if (auto clientId = header->clientId()) {
      header->setHeader(transport::THeader::kClientId, std::move(*clientId));
    }

    if (auto serviceTraceMeta = header->serviceTraceMeta()) {
      header->setHeader(
          transport::THeader::kServiceTraceMeta, std::move(*serviceTraceMeta));
    }

    if (auto tenantId = header->tenantId()) {
      header->setHeader(transport::THeader::kTenantId, std::move(*tenantId));
    }
  }
}

void HeaderChannel::preprocessHeader(
    apache::thrift::transport::THeader* header) {
  header->mutableWriteHeaders().insert(
      persistentWriteHeaders_.begin(), persistentWriteHeaders_.end());

  if (compressionConfig_ && !header->getDesiredCompressionConfig()) {
    header->setDesiredCompressionConfig(*compressionConfig_);
  }

  if (auto& writeTransforms = header->getWriteTransforms();
      !writeTransforms.empty()) {
    DCHECK(writeTransforms.size() == 1);
    auto transform = writeTransforms.back();
    writeTransforms.clear();

    if (!header->getDesiredCompressionConfig()) {
      apache::thrift::CompressionConfig compressionConfig;
      switch (transform) {
        case transport::THeader::ZLIB_TRANSFORM: {
          apache::thrift::CodecConfig codec;
          codec.zlibConfig_ref() = apache::thrift::ZlibCompressionCodecConfig();
          compressionConfig.codecConfig_ref() = codec;
          break;
        }
        case transport::THeader::ZSTD_TRANSFORM: {
          apache::thrift::CodecConfig codec;
          codec.zstdConfig_ref() = apache::thrift::ZstdCompressionCodecConfig();
          compressionConfig.codecConfig_ref() = codec;
          break;
        }
        default:
          LOG(DFATAL) << "Unsupported transform: " << transform;
      }
      header->setDesiredCompressionConfig(compressionConfig);
    }
  }
}
} // namespace thrift
} // namespace apache
