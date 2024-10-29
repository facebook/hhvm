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

#include <folly/compression/Compression.h>

#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

class CompressionAlgorithmSelector {
 public:
  static CompressionAlgorithm fromCodecConfig(const CodecConfig& codecConfig);

  static std::pair<folly::io::CodecType, int> toCodecTypeAndLevel(
      const CompressionAlgorithm& compressionAlgorithm);
};

} // namespace apache::thrift::rocket
