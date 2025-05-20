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
#include <optional>
#include <string>

#include <folly/compression/Compression.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

/**
 * TBD.
 */
class CompressionManager {
 public:
  virtual ~CompressionManager() = default;

  CompressionAlgorithm fromCodecConfig(const CodecConfig& codecConfig);

  std::pair<folly::compression::CodecType, int> toCodecTypeAndLevel(
      const CompressionAlgorithm& compressionAlgorithm);

  void setCompressionCodec(
      CompressionConfig compressionConfig,
      RequestRpcMetadata& metadata,
      size_t payloadSize);

  void setCompressionCodec(
      CompressionConfig compressionConfig,
      ResponseRpcMetadata& metadata,
      size_t payloadSize);

  void setCompressionCodec(
      CompressionConfig compressionConfig,
      StreamPayloadMetadata& metadata,
      size_t payloadSize);

  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm);

  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm);
};

} // namespace apache::thrift::rocket
